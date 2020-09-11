#pragma once

#include <cmath>
#include <cstdlib>
#include <iostream>

// Copied over from util.cpp so there is no link required.
inline std::string
BytePrettyPrint(size_t bytes) {
  auto base = (double)std::log(bytes) / (double)std::log(1024);
  const char* suffixArr[] = { "B", "kiB", "MiB", "GiB", "TiB", "PiB" };
  return std::to_string(
           (size_t)std::round(std::pow(1024, base - std::floor(base)))) +
         suffixArr[(size_t)std::floor(base)] + " (=" + std::to_string(bytes) +
         " B)";
}
