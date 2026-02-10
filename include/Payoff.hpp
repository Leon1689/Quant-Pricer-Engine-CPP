#pragma once

enum class OptionType { Call, Put };

class Payoff {
public:
  virtual double operator()(double spot) const = 0;
  virtual ~Payoff() = default;
};

class PayoffCall : public Payoff {
public:
  explicit PayoffCall(double strike);
  double operator()(double spot) const override;

private:
  double strike_;
};

class PayoffPut : public Payoff {
public:
  explicit PayoffPut(double strike);
  double operator()(double spot) const override;

private:
  double strike_;
};
