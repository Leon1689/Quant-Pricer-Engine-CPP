#include "RandomGen.hpp"
#include <chrono>
#include <random>

// Thread-local random engine to avoid locking overhead in OpenMP
// Seeded with a combination of random_device and time to ensure uniqueness
thread_local std::mt19937_64 generator = [] {
  uint64_t seed =
      std::random_device{}() ^
      std::chrono::high_resolution_clock::now().time_since_epoch().count();
  return std::mt19937_64(seed);
}();

RandomGen::RandomGen(DistributionType type, double df) : type_(type), df_(df) {}

void RandomGen::generate(std::vector<double> &randoms, size_t count) {
  randoms.resize(count);
  if (type_ == DistributionType::Normal) {
    std::normal_distribution<double> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
      randoms[i] = dist(generator);
    }
  } else {
    std::student_t_distribution<double> dist(df_);
    for (size_t i = 0; i < count; ++i) {
      randoms[i] = dist(generator);
    }
  }
}

double RandomGen::next() {
  if (type_ == DistributionType::Normal) {
    std::normal_distribution<double> dist(0.0, 1.0);
    return dist(generator);
  } else {
    std::student_t_distribution<double> dist(df_);
    return dist(generator);
  }
}
