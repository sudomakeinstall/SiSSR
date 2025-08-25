#ifndef sissr_Utils_h
#define sissr_Utils_h

// System
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace sissr {

// String utilities
inline std::string ensureTrailingSlash(const std::string& path) {
    return path.empty() || path.back() != '/' ? path + '/' : path;
}

// Timestamp generation
inline std::string getCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    return ss.str();
}

// Math utilities
template<typename T>
inline bool isBetween(T value, T min, T max) {
    return value >= min && value <= max;
}

template<typename T>
inline bool close(T a, T b, T tolerance = T(1e-9)) {
    return std::abs(a - b) <= tolerance;
}

} // namespace sissr

#endif