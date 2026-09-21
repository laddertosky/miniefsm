#include "miniefsm.hpp"

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
  static constexpr MyState From = MyState::Idle;
  static constexpr MyState To = MyState::Waiting;
  static bool Guard(const MyInput &input, const MyContext &ctx) {
    return input.in == 0;
  }
  static void Action(const MyInput &input, MyContext &ctx, MyOutput &output) {
    ctx.buf = input.in;
    ctx.clock = 0;
  }
};

struct GenerateOutput {
  static constexpr MyState From = MyState::Waiting;
  static constexpr MyState To = MyState::Idle;
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
  using InitialState = IdleState;
  using Input = MyInput;
  using Output = MyOutput;
  using Context = MyContext;
  using States = miniefsm::TypeList<IdleState, WaitingState>;
  using Transitions = miniefsm::TypeList<StartWaiting, GenerateOutput>;
};

int main(int argc, char **argv) {}
