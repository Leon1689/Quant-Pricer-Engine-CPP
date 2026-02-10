#include "Engine.hpp"
#include "Xoshiro.hpp"
#include <cmath>
#include <numeric>
#include <random>
#include <thread> // Added for std::thread
#include <vector>

// Helper to get thread ID or return 0 if no OpenMP
int get_thread_num() {
#ifdef _OPENMP
  return omp_get_thread_num();
#else
  return 0; // Not used in std::thread implementation below
#endif
}

Engine::Engine(const EngineParams &params, const Payoff &payoff, RandomGen &rng)
    : params_(params), payoff_(payoff), rng_(rng) {}

// Utility to generate random numbers into a block
template <typename RNG, typename Dist>
void generate_block(RNG &rng, Dist &dist, std::vector<double> &buffer,
                    size_t count) {
  for (size_t i = 0; i < count; ++i) {
    buffer[i] = dist(rng);
  }
}

double Engine::calculatePrice() const {
  // Pre-calculate constants
  double dt = params_.expiry / params_.steps;
  double sqrt_dt = std::sqrt(dt);
  double drift_step =
      (params_.rate - 0.5 * params_.volatility * params_.volatility) * dt;
  double vol_step = params_.volatility * sqrt_dt;
  double discount = std::exp(-params_.rate * params_.expiry);

  double total_payoff = 0.0;
  double df = rng_.get_df(); // Use dynamic DF from the passed RNG setup

  // Dynamic seeding: hardware entropy ^ time
  // This base seed is mixed with thread ID to ensure unique streams
  uint64_t base_seed =
      std::random_device{}() ^
      std::chrono::high_resolution_clock::now().time_since_epoch().count();

// Parallel Monte Carlo Simulation using OpenMP
// reduction(+:total_payoff) efficiently sums results without explicit locking
#pragma omp parallel reduction(+ : total_payoff)
  {
    // Thread-local RNG initialization
    // Crucial: base_seed + omp_get_thread_num() prevents cross-thread path
    // correlation, ensuring statistical independence of the simulated paths.
    int thread_id = 0;
#ifdef _OPENMP
    thread_id = omp_get_thread_num();
#endif
    Xoshiro256PlusPlus rng(base_seed + thread_id);

    std::normal_distribution<double> normal_dist(0.0, 1.0);
    std::student_t_distribution<double> t_dist(df);

    // Block-based processing for L1/L2 cache efficiency
    constexpr size_t BLOCK_SIZE = 1024;
    std::vector<double> spots(BLOCK_SIZE);
    std::vector<double> rands(BLOCK_SIZE);

#pragma omp for schedule(static)
    for (long long i = 0; i < static_cast<long long>(params_.paths);
         i += BLOCK_SIZE) {
      size_t current_block_size =
          std::min(static_cast<size_t>(BLOCK_SIZE),
                   static_cast<size_t>(params_.paths - i));

      // Initialize spots for this block
      std::fill(spots.begin(), spots.begin() + current_block_size,
                params_.spot);

      // Time-stepping loop (252 steps)
      // Optimized for CPU cache by keeping 'spots' hot in cache
      for (size_t step = 0; step < params_.steps; ++step) {
        if (params_.distType == DistributionType::Normal) {
          for (size_t k = 0; k < current_block_size; ++k)
            rands[k] = normal_dist(rng);
        } else {
          for (size_t k = 0; k < current_block_size; ++k)
            rands[k] = t_dist(rng);
        }

        for (size_t k = 0; k < current_block_size; ++k) {
          spots[k] *= std::exp(drift_step + vol_step * rands[k]);
        }
      }

      // Calculate payoff for the block and accumulate
      double block_payoff = 0.0;
      for (size_t k = 0; k < current_block_size; ++k) {
        block_payoff += payoff_(spots[k]);
      }
      total_payoff += block_payoff;
    }
  }

  return (total_payoff / params_.paths) * discount;
}

PricingResult Engine::calculateAll() const {
  auto start_time = std::chrono::high_resolution_clock::now();

  // Dynamic seeding: hardware entropy ^ time
  uint64_t base_seed =
      std::random_device{}() ^
      std::chrono::high_resolution_clock::now().time_since_epoch().count();

  double bump = 0.01 * params_.spot;
  double S0 = params_.spot;
  double S_up = S0 + bump;
  double S_down = S0 - bump;

  double dt = params_.expiry / params_.steps;
  double sqrt_dt = std::sqrt(dt);
  double drift_step =
      (params_.rate - 0.5 * params_.volatility * params_.volatility) * dt;
  double vol_step = params_.volatility * sqrt_dt;
  double discount = std::exp(-params_.rate * params_.expiry);

  double total_price = 0.0;
  double total_up = 0.0;
  double total_down = 0.0;
  double df = rng_.get_df();

// Parallel Monte Carlo Simulation using OpenMP
#pragma omp parallel reduction(+ : total_price, total_up, total_down)
  {
    int thread_id = 0;
#ifdef _OPENMP
    thread_id = omp_get_thread_num();
#endif
    Xoshiro256PlusPlus rng(base_seed + thread_id);

    std::normal_distribution<double> normal_dist(0.0, 1.0);
    std::student_t_distribution<double> t_dist(df);

    constexpr size_t BLOCK_SIZE = 1024;
    std::vector<double> spots(BLOCK_SIZE);
    std::vector<double> spots_up(BLOCK_SIZE);
    std::vector<double> spots_down(BLOCK_SIZE);
    std::vector<double> rands(BLOCK_SIZE);

#pragma omp for schedule(static)
    for (long long i = 0; i < static_cast<long long>(params_.paths);
         i += BLOCK_SIZE) {
      size_t current_block_size =
          std::min(static_cast<size_t>(BLOCK_SIZE),
                   static_cast<size_t>(params_.paths - i));

      std::fill(spots.begin(), spots.begin() + current_block_size, S0);
      std::fill(spots_up.begin(), spots_up.begin() + current_block_size, S_up);
      std::fill(spots_down.begin(), spots_down.begin() + current_block_size,
                S_down);

      for (size_t step = 0; step < params_.steps; ++step) {
        if (params_.distType == DistributionType::Normal) {
          for (size_t k = 0; k < current_block_size; ++k)
            rands[k] = normal_dist(rng);
        } else {
          for (size_t k = 0; k < current_block_size; ++k)
            rands[k] = t_dist(rng);
        }

        for (size_t k = 0; k < current_block_size; ++k) {
          double shock = std::exp(drift_step + vol_step * rands[k]);
          spots[k] *= shock;
          spots_up[k] *= shock;
          spots_down[k] *= shock;
        }
      }

      double block_price = 0.0;
      double block_up = 0.0;
      double block_down = 0.0;

      for (size_t k = 0; k < current_block_size; ++k) {
        block_price += payoff_(spots[k]);
        block_up += payoff_(spots_up[k]);
        block_down += payoff_(spots_down[k]);
      }
      total_price += block_price;
      total_up += block_up;
      total_down += block_down;
    }
  }

  double price = (total_price / params_.paths) * discount;
  double price_up = (total_up / params_.paths) * discount;
  double price_down = (total_down / params_.paths) * discount;

  double delta = (price_up - price_down) / (2 * bump);
  double gamma = (price_up - 2 * price + price_down) / (bump * bump);

  auto end_time = std::chrono::high_resolution_clock::now();
  double time_ms =
      std::chrono::duration<double, std::milli>(end_time - start_time).count();

  return {price, delta, gamma, time_ms, base_seed};
}
