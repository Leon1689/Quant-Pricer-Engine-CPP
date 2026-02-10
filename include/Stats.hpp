#pragma once
#include <cstdint>

struct PricingResult {
  double price;
  double delta;
  double gamma;
  double time_ms;
  uint64_t base_seed;
};
