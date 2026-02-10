#pragma once
#include <algorithm>

#include <thread>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace ParallelUtils {

template <typename Function>
void ParallelFor(long long start, long long end, Function func) {
#ifdef _OPENMP
#pragma omp parallel for
  for (long long i = start; i < end; ++i) {
    func(i);
  }
#else
  unsigned int num_threads = std::thread::hardware_concurrency();
  if (num_threads == 0)
    num_threads = 1;

  long long total_work = end - start;
  long long chunk_size = (total_work + num_threads - 1) / num_threads;

  std::vector<std::thread> threads;
  for (unsigned int t = 0; t < num_threads; ++t) {
    long long chunk_start = start + t * chunk_size;
    long long chunk_end = std::min(end, chunk_start + chunk_size);

    if (chunk_start < end) {
      threads.emplace_back([=]() {
        for (long long i = chunk_start; i < chunk_end; ++i) {
          func(i);
        }
      });
    }
  }

  for (auto &thread : threads) {
    if (thread.joinable()) {
      thread.join();
    }
  }
#endif
}
} // namespace ParallelUtils
