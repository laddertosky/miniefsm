#include "miniefsm.hpp"
#include "typelist.hpp"
#include <catch2/catch_test_macros.hpp>

using miniefsm::TypeList;

// ---------------------------------------------------------------------
// Machine #1: mirrors main.cpp's Idle/Waiting example.
// ---------------------------------------------------------------------
namespace idle_waiting {

enum class MyState { Idle, Waiting };
struct MyInput {
  int in;
};
struct MyOutput {
  int out;
};
struct MyContext {
  int clock;
  int buf;
};

struct IdleState {
  static constexpr MyState Id = MyState::Idle;
  static bool Invariant(const MyContext &) { return true; }
};
struct WaitingState {
  static constexpr MyState Id = MyState::Waiting;
  static bool Invariant(const MyContext &ctx) { return ctx.clock <= 3; }
};

struct StartWaiting {
  using From = IdleState;
  using To = WaitingState;
  static bool Guard(const MyInput &input, const MyContext &) {
    return input.in == 0;
  }
  static void Action(const MyInput &input, MyContext &ctx, MyOutput &) {
    ctx.buf = input.in;
    ctx.clock = 0;
  }
};
struct GenerateOutput {
  using From = WaitingState;
  using To = IdleState;
  static bool Guard(const MyInput &, const MyContext &ctx) {
    return ctx.clock >= 3;
  }
  static void Action(const MyInput &, MyContext &ctx, MyOutput &output) {
    output.out = ctx.buf;
    ctx.buf = 0;
  }
};

struct MyMachineDefinition {
  using StateEnum = MyState;
  using Input = MyInput;
  using Output = MyOutput;
  using Context = MyContext;
  using States = TypeList<IdleState, WaitingState>;
  using Transitions = TypeList<StartWaiting, GenerateOutput>;
  static constexpr auto InitialState = IdleState{};
};

} // namespace idle_waiting

TEST_CASE("Machine starts in the declared InitialState") {
  using namespace idle_waiting;
  miniefsm::Machine<MyMachineDefinition> m;
  REQUIRE(m.CurrentStateIs<IdleState>());
}

TEST_CASE("Machine transitions Idle -> Waiting when the guard passes") {
  using namespace idle_waiting;
  miniefsm::Machine<MyMachineDefinition> m;
  MyOutput out{};
  bool fired = m.Step(MyInput{.in = 0}, out);
  REQUIRE(fired);
  REQUIRE(m.CurrentStateIs<WaitingState>());
}

TEST_CASE("Machine does not fire when the guard's condition isn't met") {
  using namespace idle_waiting;
  miniefsm::Machine<MyMachineDefinition> m;
  MyOutput out{};

  // StartWaiting::Guard requires input.in == 0, so a non-zero input should
  // not trigger a transition; the machine should stay in IdleState.
  bool fired = m.Step(MyInput{.in = 7}, out);
  REQUIRE_FALSE(fired);
  REQUIRE(m.CurrentStateIs<IdleState>());
}

TEST_CASE("Machine stays in Waiting while clock never reaches 3") {
  using namespace idle_waiting;
  miniefsm::Machine<MyMachineDefinition> m;
  MyOutput out{};
  REQUIRE(m.Step(MyInput{.in = 0}, out)); // Idle -> Waiting, clock reset to 0

  // Nothing in this example machine increments ctx.clock over time (only
  // StartWaiting's Action resets it to 0), so GenerateOutput's guard
  // (clock >= 3) can never become true here. This is the "normal,
  // expected" no-match case discussed earlier, not an error: the machine
  // is correctly waiting, not stuck due to a bug.
  for (int i = 0; i < 5; ++i) {
    REQUIRE_FALSE(m.Step(MyInput{.in = 0}, out));
  }
  REQUIRE(m.CurrentStateIs<WaitingState>());
}

TEST_CASE("StartWaiting's Action correctly records the input into context") {
  using namespace idle_waiting;
  miniefsm::Machine<MyMachineDefinition> m;
  MyOutput out{};
  REQUIRE(m.Step(MyInput{.in = 0}, out)); // guard only accepts in == 0
  REQUIRE(m.GetContext().buf == 0);
  REQUIRE(m.GetContext().clock == 0);
}

// ---------------------------------------------------------------------
// Machine #2: deliberately different shape (three states, three
// transitions, a PriorityPolicy) to make sure nothing in the library is
// accidentally hardcoded to two states/two transitions.
// ---------------------------------------------------------------------
namespace traffic_light {

enum class LightState { Red, Yellow, Green };
struct Tick {};
struct NoOutput {};
struct LightContext {
  int elapsed = 0;
};

struct RedState {
  static constexpr LightState Id = LightState::Red;
  static bool Invariant(const LightContext &) { return true; }
};
struct YellowState {
  static constexpr LightState Id = LightState::Yellow;
  static bool Invariant(const LightContext &) { return true; }
};
struct GreenState {
  static constexpr LightState Id = LightState::Green;
  static bool Invariant(const LightContext &) { return true; }
};

struct RedToGreen {
  using From = RedState;
  using To = GreenState;
  static bool Guard(const Tick &, const LightContext &ctx) {
    return ctx.elapsed >= 2;
  }
  static void Action(const Tick &, LightContext &ctx, NoOutput &) {
    ctx.elapsed = 0;
  }
};
struct GreenToYellow {
  using From = GreenState;
  using To = YellowState;
  static bool Guard(const Tick &, const LightContext &ctx) {
    return ctx.elapsed >= 3;
  }
  static void Action(const Tick &, LightContext &ctx, NoOutput &) {
    ctx.elapsed = 0;
  }
};
struct YellowToRed {
  using From = YellowState;
  using To = RedState;
  static bool Guard(const Tick &, const LightContext &ctx) {
    return ctx.elapsed >= 1;
  }
  static void Action(const Tick &, LightContext &ctx, NoOutput &) {
    ctx.elapsed = 0;
  }
};

struct TrafficLightDefinition {
  using StateEnum = LightState;
  using Input = Tick;
  using Output = NoOutput;
  using Context = LightContext;
  using States = TypeList<RedState, YellowState, GreenState>;
  using Transitions = TypeList<RedToGreen, GreenToYellow, YellowToRed>;
  static constexpr auto InitialState = RedState{};
};

} // namespace traffic_light

TEST_CASE("A three-state machine with a different Policy also works") {
  using namespace traffic_light;
  // elapsed never actually increments in this toy example (no tick-counter
  // logic in Action), so the guard requiring elapsed >= 2 never passes;
  // this test only checks that a differently-shaped Definition + Policy
  // combination compiles and runs without firing, i.e. nothing in the
  // library assumes two states/two transitions.
  miniefsm::Machine<TrafficLightDefinition, miniefsm::PriorityPolicy> m;
  REQUIRE(m.CurrentStateIs<RedState>());
  NoOutput out{};
  bool fired = m.Step(Tick{}, out);
  (void)fired;
  REQUIRE(m.CurrentStateIs<RedState>());
}
