#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>

template <typename Policy, std::size_t N>
concept BasePolicy = requires(const std::array<bool, N> &matches) {
  { Policy::Select(matches) } -> std::same_as<std::optional<std::size_t>>;
};

template <typename Policy, std::size_t N>
concept PriorityAwarePolicy = requires(const std::array<bool, N> &matches,
                                       const std::array<int, N> &priorities) {
  {
    Policy::Select(matches, priorities)
  } -> std::same_as<std::optional<std::size_t>>;
};

template <typename Policy, std::size_t N>
concept ValidPolicy = BasePolicy<Policy, N> || PriorityAwarePolicy<Policy, N>;

struct FirstMatchPolicy {
  template <std::size_t N>
  static std::optional<std::size_t> Select(const std::array<bool, N> &matches) {
    for (std::size_t i = 0; i < N; i++) {
      if (matches[i])
        return i;
    }

    return std::nullopt;
  }
};

struct LastMatchPolicy {
  template <std::size_t N>
  static std::optional<std::size_t> Select(const std::array<bool, N> &matches) {
    for (std::size_t i = N - 1; i >= 0; i--) {
      if (matches[i])
        return i;
    }
    return std::nullopt;
  }
};

struct StrictOnePolicy {
  template <std::size_t N>
  static std::optional<std::size_t> Select(const std::array<bool, N> &matches) {
    std::optional<std::size_t> found;
    for (std::size_t i = 0; i < N; ++i) {
      if (matches[i]) {
        if (found)
          throw std::logic_error("Ambiguous transition guards");
        found = i;
      }
    }
    return found;
  }
};

struct PriorityPolicy {
  template <std::size_t N>
  static std::optional<std::size_t>
  Select(const std::array<bool, N> &matches,
         const std::array<int, N> &priorities) {

    std::optional<std::size_t> best;
    for (std::size_t i = 0; i < N; ++i) {
      if (!matches[i])
        continue;
      if (!best.has_value() || priorities[i] > priorities[best.value()]) {
        best = i;
      }
    }
    return best;
  }
};
