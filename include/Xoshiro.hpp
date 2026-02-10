#pragma once
#include <cstdint>
#include <limits>

// Xoshiro256++ Random Number Generator
// Extremely fast, high-quality PRNG suitable for large-scale Monte Carlo
// simulations. Not cryptographically secure, but perfect for physics/finance
// simulations.
class Xoshiro256PlusPlus {
public:
  using result_type = uint64_t;

  explicit Xoshiro256PlusPlus(uint64_t seed = 0) {
    // SplitMix64 to initialize state from a single seed
    uint64_t z = (seed + 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    s[0] = z ^ (z >> 31);

    z = (s[0] + 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    s[1] = z ^ (z >> 31);

    z = (s[1] + 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    s[2] = z ^ (z >> 31);

    z = (s[2] + 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    s[3] = z ^ (z >> 31);
  }

  static constexpr uint64_t min() {
    return std::numeric_limits<uint64_t>::min();
  }
  static constexpr uint64_t max() {
    return std::numeric_limits<uint64_t>::max();
  }

  uint64_t operator()() {
    const uint64_t result = rotl(s[0] + s[3], 23) + s[0];

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    return result;
  }

  // Jump function to skip 2^128 calls (useful for parallel streams)
  void jump() {
    static const uint64_t JUMP[] = {0x180ec6d33cfd0ebb, 0xd5a61266f0c9392c,
                                    0xa9582618e03fc9aa, 0x39abdc4529b1661c};

    uint64_t s0 = 0;
    uint64_t s1 = 0;
    uint64_t s2 = 0;
    uint64_t s3 = 0;

    for (size_t i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
      for (int b = 0; b < 64; b++) {
        if (JUMP[i] & (1ULL << b)) {
          s0 ^= s[0];
          s1 ^= s[1];
          s2 ^= s[2];
          s3 ^= s[3];
        }
        operator()();
      }

    s[0] = s0;
    s[1] = s1;
    s[2] = s2;
    s[3] = s3;
  }

private:
  uint64_t s[4];

  static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
  }
};
