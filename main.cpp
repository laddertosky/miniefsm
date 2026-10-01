#include "miniefsm.hpp"
#include "typelist.hpp"
#include <cassert>

enum class MyState {
  Idle,
  Waiting,
};

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
  static bool Invariant(const MyContext &ctx) { return true; }
};

struct WaitingState {
  static constexpr MyState Id = MyState::Waiting;
  static bool Invariant(const MyContext &ctx) { return ctx.clock <= 3; }
};

struct StartWaiting {
  using From = IdleState;
  using To = WaitingState;

  static bool Guard(const MyInput &input, const MyContext &ctx) {
    return ctx.buf == 0;
  }
  static void Action(const MyInput &input, MyContext &ctx, MyOutput &output) {
    ctx.buf = input.in;
    ctx.clock = 0;
  }
};

struct GenerateOutput {
  using From = WaitingState;
  using To = IdleState;

  static bool Guard(const MyInput &input, const MyContext &ctx) {
    return ctx.clock >= 3;
  }
  static void Action(const MyInput &input, MyContext &ctx, MyOutput &output) {
    output.out = ctx.buf;
    ctx.buf = 0;
  }
};

struct MyMachineDefinition {
  using StateEnum = MyState;
  using Input = MyInput;
  using Output = MyOutput;
  using Context = MyContext;
  using States = miniefsm::TypeList<IdleState, WaitingState>;
  using Transitions = miniefsm::TypeList<StartWaiting, GenerateOutput>;

  static constexpr auto InitialState = IdleState{};
};

int main(int argc, char **argv) {
  miniefsm::Machine<MyMachineDefinition> machine;

  MyOutput out{};
  assert(machine.CurrentStateIs<IdleState>());
  machine.Step(MyInput{.in = 1}, out);
  assert(machine.CurrentStateIs<WaitingState>());
}
