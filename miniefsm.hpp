#include <concepts>

namespace miniefsm {

template <typename State, typename StateEnum, typename Context>
concept StateDefinition =
    // Members
    requires { requires std::same_as<StateEnum, decltype(State::Id)>; } &&

    // Methods
    requires(const Context &ctx) {
      { State::Invariant(ctx) } -> std::same_as<bool>;
    };

template <typename... Types> struct TypeList {};

template <typename Transition, typename StateEnum, typename Input,
          typename Context, typename Output>
concept TransitionDefinition =
    // Members
    requires {
      requires std::same_as<decltype(Transition::From), StateEnum>;
      requires std::same_as<decltype(Transition::To), StateEnum>;
    } &&

    // Methods
    requires(const Input &input, const Context &readCtx, Context &writeCtx,
             Output &output) {
      { Transition::Guard(input, readCtx) } -> std::same_as<bool>;
      { Transition::Action(input, writeCtx, output) } -> std::same_as<void>;
    };

template <typename StateEnum, typename Context, typename... States>
concept ValidStates = (StateDefinition<States, StateEnum, Context> && ...);

template <typename StateEnum, typename Input, typename Context, typename Output,
          typename... Transitions>
concept ValidTransitions =
    (TransitionDefinition<Transitions, StateEnum, Input, Context, Output> &&
     ...);

template <typename Definition>
concept MachineDefinition =
    // Members
    requires {
      typename Definition::StateEnum;
      typename Definition::InitialState;
      typename Definition::Input;
      typename Definition::Context;
      typename Definition::Output;

      typename Definition::States;
      typename Definition::Transitions;
    } &&

    // Consistency Checks
    ValidStates<typename Definition::StateEnum, typename Definition::Context,
                typename Definition::States> &&

    ValidTransitions<typename Definition::StateEnum, typename Definition::Input,
                     typename Definition::Context, typename Definition::Output,
                     typename Definition::Transitions> &&

    requires {
      requires std::same_as<decltype(Definition::InitialState),
                            typename Definition::StateEnum>;
    };

template <typename State, typename Transitions, typename Policy>
concept SelectionPolicy =
    requires(const Policy &policy, const State &currentState,
             const Transitions &candidates) {
      { policy.RunOne(currentState, candidates) } -> std::same_as<State>;
    };

template <typename Definition, typename Policy>
  requires MachineDefinition<Definition> &&
           SelectionPolicy<typename Definition::InitialState,
                           typename Definition::Transitions, Policy>
class Machine {
  using State = typename Definition::InitialState;
  using Input = typename Definition::Input;
  using Output = typename Definition::Output;
  using Context = typename Definition::Context;

  using States = typename Definition::States;
  using Transitions = typename Definition::Transitions;

public:
  void Run() {
    while (true) {
      currentState = Policy::RunOne(currentState, transitions);
    }
  }

private:
  State currentState = Definition::InitialState;
  static States states = Definition::States;
  static Transitions transitions = Definition::Transitions;
};

} // namespace miniefsm
