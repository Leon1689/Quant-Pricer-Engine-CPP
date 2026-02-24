#include "Engine.hpp"
#include "Payoff.hpp"
#include "RandomGen.hpp"
#include <iomanip>
#include <iostream>
#include <string>

PricingResult run_benchmark(const std::string &name, DistributionType distType,
                            size_t paths, size_t steps = 252) {
  double S0 = 100.0;
  double K = 100.0;
  double r = 0.05;
  double sigma = 0.2;
  double T = 1.0;

  // Aggregate initialization with all fields (C++17)
  EngineParams params{S0, sigma, r, T, paths, steps, distType};

  // RandomGen is still passed but might be ignored by optimized Engine
  RandomGen rng(distType, 4.0);
  PayoffCall payoff(K);
  Engine engine(params, payoff, rng);

  std::cout << "Running " << name << " (" << steps << " steps) with " << paths
            << " paths..." << std::endl;
  PricingResult result = engine.calculateAll();

  std::cout << std::fixed << std::setprecision(5);
  std::cout << "Base Seed: " << result.base_seed << std::endl;
  std::cout << "Price: " << result.price << std::endl;
  std::cout << "Delta: " << result.delta << std::endl;
  std::cout << "Gamma: " << result.gamma << std::endl;
  std::cout << "Time:  " << result.time_ms << " ms" << std::endl;
  double throughput = (static_cast<double>(paths) / result.time_ms) * 1000.0;
  std::cout << "Throughput: " << static_cast<long long>(throughput)
            << " paths/sec" << std::endl;
  std::cout << "----------------------------------------" << std::endl;
  return result;
}

int main() {
  std::cout << "Quant Pricer Engine Benchmark (Multi-Step Euler-Maruyama)"
            << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  // Warmup
  run_benchmark("Warmup (Normal)", DistributionType::Normal, 10000);

  // Normal Distribution
  PricingResult res_normal =
      run_benchmark("Normal Distribution", DistributionType::Normal,
                    1000000); // 1M paths

  // Fat-Tail (Student's t)
  PricingResult res_fat =
      run_benchmark("Fat-Tail (Student-t, df=4)", DistributionType::StudentT,
                    1000000); // 1M paths

  // Model Risk Premium
  double risk_premium = res_fat.price - res_normal.price;
  std::cout << ">>> Model Risk Premium (Fat-Tail - Normal): " << risk_premium
            << std::endl;
  std::cout << "----------------------------------------" << std::endl;

  // Scalability Test (10M paths)
  // run_benchmark("High Load (Normal, 10M paths)", DistributionType::Normal,
  // 10000000); // Optional, might be slow

  return 0;
}
