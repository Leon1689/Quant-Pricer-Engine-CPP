#include "Payoff.hpp"
#include <algorithm>

PayoffCall::PayoffCall(double strike) : strike_(strike) {}

double PayoffCall::operator()(double spot) const {
  return std::max(spot - strike_, 0.0);
}

PayoffPut::PayoffPut(double strike) : strike_(strike) {}

double PayoffPut::operator()(double spot) const {
  return std::max(strike_ - spot, 0.0);
}
