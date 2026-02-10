#pragma once

#include <vector>

enum class DistributionType { Normal, StudentT };

class RandomGen {
public:
  RandomGen(DistributionType type, double df = 5.0);

  // Generate a vector of random numbers
  void generate(std::vector<double> &randoms, size_t count);

  // Get a single random number (thread-safe if engine is thread_local)
  double next();

  // Get df
  double get_df() const { return df_; }

private:
  DistributionType type_;
  double df_; // Degrees of freedom for Student's t
};
