#include "policy.hpp"
#include <array>
#include <catch2/catch_test_macros.hpp>

using miniefsm::FirstMatchPolicy;
using miniefsm::PriorityPolicy;
using miniefsm::StrictOnePolicy;

TEST_CASE("FirstMatchPolicy picks the earliest true index") {
  std::array<bool, 3> matches{false, true, true};
  auto result = FirstMatchPolicy::Select(matches);
  REQUIRE(result.has_value());
  REQUIRE(*result == 1);
}

TEST_CASE("FirstMatchPolicy returns nullopt when nothing matches") {
  std::array<bool, 3> matches{false, false, false};
  auto result = FirstMatchPolicy::Select(matches);
  REQUIRE_FALSE(result.has_value());
}

TEST_CASE(
    "FirstMatchPolicy: index 0 matching is not confused with 'no match'") {
  std::array<bool, 2> matches{true, false};
  auto result = FirstMatchPolicy::Select(matches);
  REQUIRE(result.has_value());
  REQUIRE(*result == 0);
}

TEST_CASE("ErrorOnAmbiguousPolicy returns the single match when unambiguous") {
  std::array<bool, 3> matches{false, true, false};
  auto result = StrictOnePolicy::Select(matches);
  REQUIRE(result.has_value());
  REQUIRE(*result == 1);
}

TEST_CASE("ErrorOnAmbiguousPolicy returns nullopt when nothing matches") {
  std::array<bool, 3> matches{false, false, false};
  auto result = StrictOnePolicy::Select(matches);
  REQUIRE_FALSE(result.has_value());
}

TEST_CASE("ErrorOnAmbiguousPolicy throws when two guards pass simultaneously") {
  std::array<bool, 3> matches{true, true, false};
  REQUIRE_THROWS_AS(StrictOnePolicy::Select(matches), std::logic_error);
}

TEST_CASE("PriorityPolicy picks the highest-priority passing slot") {
  std::array<bool, 3> matches{true, true, false};
  std::array<int, 3> priorities{
      0, 10, 99}; // slot 2 has highest priority but didn't match
  auto result = PriorityPolicy::Select(matches, priorities);
  REQUIRE(result.has_value());
  REQUIRE(*result == 1); // slot 1 beats slot 0 on priority
}

TEST_CASE("PriorityPolicy breaks ties by keeping the earliest slot") {
  std::array<bool, 3> matches{true, true, true};
  std::array<int, 3> priorities{5, 5, 5}; // all tied
  auto result = PriorityPolicy::Select(matches, priorities);
  REQUIRE(result.has_value());
  REQUIRE(*result == 0);
}

TEST_CASE("PriorityPolicy returns nullopt when nothing matches") {
  std::array<bool, 3> matches{false, false, false};
  std::array<int, 3> priorities{9, 9, 9};
  auto result = PriorityPolicy::Select(matches, priorities);
  REQUIRE_FALSE(result.has_value());
}
