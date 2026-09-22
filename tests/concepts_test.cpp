#include "miniefsm.hpp"
#include "typelist.hpp"
#include <catch2/catch_test_macros.hpp>

using miniefsm::MachineDefinition;
using miniefsm::StateDefinition;
using miniefsm::TransitionDefinition;
using miniefsm::TypeList;
using miniefsm::ValidStates;
using miniefsm::ValidTransitions;

namespace {

enum class TestState { A, B };
struct TestInput {};
struct TestOutput {};
struct TestContext {
  int value;
};

// ---- StateDefinition: valid case ----
struct GoodState {
  static constexpr TestState Id = TestState::A;
  static bool Invariant(const TestContext &) { return true; }
};
static_assert(StateDefinition<GoodState, TestState, TestContext>);

// ---- StateDefinition: missing Invariant ----
struct MissingInvariant {
  static constexpr TestState Id = TestState::A;
};
static_assert(!StateDefinition<MissingInvariant, TestState, TestContext>);

// ---- StateDefinition: wrong Id type ----
struct WrongIdType {
  static constexpr int Id = 0; // not TestState
  static bool Invariant(const TestContext &) { return true; }
};
static_assert(!StateDefinition<WrongIdType, TestState, TestContext>);

// ---- StateDefinition: Invariant with wrong return type ----
struct WrongInvariantReturn {
  static constexpr TestState Id = TestState::A;
  static int Invariant(const TestContext &) { return 1; } // not bool
};
static_assert(!StateDefinition<WrongInvariantReturn, TestState, TestContext>);

struct StateA {
  static constexpr TestState Id = TestState::A;
  static bool Invariant(const TestContext &) { return true; }
};
struct StateB {
  static constexpr TestState Id = TestState::B;
  static bool Invariant(const TestContext &) { return true; }
};

// ---- TransitionDefinition: valid case ----
struct GoodTransition {
  using From = StateA;
  using To = StateB;
  static bool Guard(const TestInput &, const TestContext &) { return true; }
  static void Action(const TestInput &, TestContext &, TestOutput &) {}
};
static_assert(TransitionDefinition<GoodTransition, TestState, TestInput,
                                   TestContext, TestOutput>);

// ---- TransitionDefinition: From/To are enum values, not state types ----
// (This is exactly the bug we hit earlier in the project: From/To must be
// types with a nested ::Id, not raw enumerators.)
struct EnumValuedTransition {
  static constexpr TestState From = TestState::A;
  static constexpr TestState To = TestState::B;
  static bool Guard(const TestInput &, const TestContext &) { return true; }
  static void Action(const TestInput &, TestContext &, TestOutput &) {}
};
static_assert(!TransitionDefinition<EnumValuedTransition, TestState, TestInput,
                                    TestContext, TestOutput>);

// ---- TransitionDefinition: missing Action ----
struct MissingAction {
  using From = StateA;
  using To = StateB;
  static bool Guard(const TestInput &, const TestContext &) { return true; }
};
static_assert(!TransitionDefinition<MissingAction, TestState, TestInput,
                                    TestContext, TestOutput>);

// ---- TransitionDefinition: Guard with wrong return type ----
struct WrongGuardReturn {
  using From = StateA;
  using To = StateB;
  static int Guard(const TestInput &, const TestContext &) {
    return 1;
  } // not bool
  static void Action(const TestInput &, TestContext &, TestOutput &) {}
};
static_assert(!TransitionDefinition<WrongGuardReturn, TestState, TestInput,
                                    TestContext, TestOutput>);

// ---- ValidStates over a TypeList ----
static_assert(ValidStates<TestState, TestContext, TypeList<StateA, StateB>>);
static_assert(
    !ValidStates<TestState, TestContext, TypeList<StateA, MissingInvariant>>);

// ---- ValidTransitions over a TypeList ----
static_assert(ValidTransitions<TestState, TestInput, TestContext, TestOutput,
                               TypeList<GoodTransition>>);
static_assert(!ValidTransitions<TestState, TestInput, TestContext, TestOutput,
                                TypeList<GoodTransition, MissingAction>>);

// ---- MachineDefinition: full valid definition ----
struct GoodDefinition {
  using StateEnum = TestState;
  using Input = TestInput;
  using Output = TestOutput;
  using Context = TestContext;
  using States = TypeList<StateA, StateB>;
  using Transitions = TypeList<GoodTransition>;
  static constexpr auto InitialState = StateA{};
};
static_assert(MachineDefinition<GoodDefinition>);

// ---- MachineDefinition: InitialState is an enum value, not a state instance
// ----
struct BadInitialState {
  using StateEnum = TestState;
  using Input = TestInput;
  using Output = TestOutput;
  using Context = TestContext;
  using States = TypeList<StateA, StateB>;
  using Transitions = TypeList<GoodTransition>;
  static constexpr TestState InitialState = TestState::A; // wrong shape
};
static_assert(!MachineDefinition<BadInitialState>);

} // namespace

// A trivial runtime test so this file shows up in `ctest` output; all the
// real checking above already happened at compile time via static_assert.
// If this file compiled at all, every static_assert above passed.
TEST_CASE("concepts_test.cpp compiled, so all static_asserts passed") {
  REQUIRE(true);
}
