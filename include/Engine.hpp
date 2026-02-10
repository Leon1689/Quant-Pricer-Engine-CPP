#pragma once
#include "Payoff.hpp"
#include "RandomGen.hpp"
#include "Stats.hpp"

struct EngineParams {
  double spot;
  double volatility;
  double rate;
  double expiry;
  size_t paths;
  size_t steps = 252; // Default to 252 steps (1 year of trading days)
  DistributionType distType = DistributionType::Normal;
};

class Engine {
public:
  Engine(const EngineParams &params, const Payoff &payoff, RandomGen &rng);

  // Calculate price
  double calculatePrice() const;

  // Calculate price and Greeks (Delta, Gamma)
  PricingResult calculateAll() const;

private:
  EngineParams params_;
  const Payoff &payoff_;
  RandomGen &rng_;
};
