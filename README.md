# High-Performance Quantitative Pricing Engine (C++17)
### Advanced Monte Carlo Simulation for Fat-Tail Risk & Derivatives Analytics

This project is a high-concurrency pricing engine optimized for modern multi-core architectures. It is designed to address the limitations of the Black-Scholes model by implementing **Fat-Tail (Student-t) distribution** models, providing more accurate risk assessment for "Black Swan" events in derivatives markets.

## 📈 Quantitative Overview
Standard models often underestimate the probability of extreme market moves (**Leptokurtosis**). This engine implements:
- **GBM Model**: For standard European-style derivatives pricing.
- **Student-t Model**: Specifically designed to capture market **Fat-Tails**, essential for high-conviction risk management and tail-risk hedging.
- **Fixed Income Extensibility**: The modular architecture allows for seamless integration of Stochastic Interest Rate models (e.g., Hull-White, Vasicek) for pricing bond options and swaps.

## 🛠 Engineering & High Performance
The engine is built with a "Performance-First" mindset, leveraging low-level C++ optimizations to achieve industrial-grade throughput.

### Performance Metrics (8-Core CPU)
- **Normal Distribution**: ~2.2 Million paths/sec
- **Student-t Distribution**: ~660,000 paths/sec
- **Scalability**: Achieved **~6.8x Speedup** through OpenMP parallelization, reaching near-linear scaling.

### Key Technical Implementations
1. **Cache-Aware Architecture**: Implemented block-based path generation (1024 paths/block) to maximize L1/L2 cache hits, minimizing memory latency during intensive simulations.
2. **Parallel Reduction (Optimized for Contention)**: Used `#pragma omp parallel reduction` instead of atomics. This eliminates "cache line bouncing" and bus contention, ensuring threads operate on local accumulators.
3. **Thread-Safe Randomness (Anti-Correlation)**: Utilized `thread_local` instances of `Xoshiro256++` with a deterministic seeding strategy (`base_seed + thread_id`). This prevents cross-thread path correlation, ensuring the statistical integrity of the Monte Carlo variance.
4. **SIMD-Friendly Design**: Structured data loops to facilitate compiler auto-vectorization, maximizing FLOPs (Floating Point Operations) per cycle.

## 🚀 Why This Matters for Trading
In a Global Markets environment like **Barrenjoey**, pricing speed directly correlates with the ability to manage real-time risk. This engine proves:
- **Accuracy**: Better modeling of extreme market distributions (Fat-Tails).
- **Agility**: Rapid "Greeks" calculation (Delta/Gamma) via Finite Difference Methods to facilitate dynamic hedging.
- **Robustness**: Production-ready C++ code that prioritizes thread safety and deterministic benchmarking.

## Build & Usage
### Prerequisites
- CMake 3.10+ | C++17 Compliant Compiler | OpenMP Support

```bash
mkdir build && cd build
cmake ..
make
./benchmark_test
