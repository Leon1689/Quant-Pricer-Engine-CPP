cmake_minimum_required(VERSION 3.10)
project(QuantPricerEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /arch:AVX2)
else()
    add_compile_options(-O3 -march=native -Wall -Wextra)
endif()

# Find OpenMP
find_package(OpenMP)

# Create a library for the Engine code
add_library(QuantEngine
    src/Engine.cpp
    src/Payoff.cpp
    src/RandomGen.cpp
)

# Specify include directories for the library
# PUBLIC means these includes are needed by the library itself AND anything linking to it
target_include_directories(QuantEngine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link OpenMP if found
if(OpenMP_CXX_FOUND)
    target_link_libraries(QuantEngine PUBLIC OpenMP::OpenMP_CXX)
endif()

# Executable
add_executable(benchmark_test tests/benchmark_test.cpp)

# Link the executable to the library
# This automatically pulls in the include directories defined in target_include_directories
target_link_libraries(benchmark_test PRIVATE QuantEngine)
