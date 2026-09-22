#pragma once

#include "typelist.hpp"
#include <concepts>
#include <type_traits>

namespace miniefsm {

template <typename State, typename StateEnum, typename Context>
concept StateDefinition =
    // Members
    requires {
      requires std::same_as<StateEnum, std::remove_cv_t<decltype(State::Id)>>;
    } &&

    // Methods
    requires(const Context &ctx) {
      { State::Invariant(ctx) } -> std::same_as<bool>;
    };

template <typename Transition, typename StateEnum, typename Input,
          typename Context, typename Output>
concept TransitionDefinition =
    // Members
    requires {
      requires std::same_as<StateEnum,
                            std::remove_cv_t<decltype(Transition::From::Id)>>;
      requires std::same_as<StateEnum,
                            std::remove_cv_t<decltype(Transition::To::Id)>>;
    } &&

    // Methods
    requires(const Input &input, const Context &readCtx, Context &writeCtx,
             Output &output) {
      { Transition::Guard(input, readCtx) } -> std::same_as<bool>;
      { Transition::Action(input, writeCtx, output) } -> std::same_as<void>;
    };

template <typename StateEnum, typename Context, typename TL>
struct ValidStateImpl; // Base case
//
template <typename StateEnum, typename Context, typename... States>
struct ValidStateImpl<StateEnum, Context, TypeList<States...>>
    : std::bool_constant<(StateDefinition<States, StateEnum, Context> && ...)> {
};

template <typename StateEnum, typename Context, typename TL>
concept ValidStates = ValidStateImpl<StateEnum, Context, TL>::value;

template <typename StateEnum, typename Input, typename Context, typename Output,
          typename TL>
struct ValidTransitionImpl; // Base case

template <typename StateEnum, typename Input, typename Context, typename Output,
          typename... Transitions>
struct ValidTransitionImpl<StateEnum, Input, Context, Output,
                           TypeList<Transitions...>>
    : std::bool_constant<(TransitionDefinition<Transitions, StateEnum, Input,
                                               Context, Output> &&
                          ...)> {};

template <typename StateEnum, typename Input, typename Context, typename Output,
          typename TL>
concept ValidTransitions =
    ValidTransitionImpl<StateEnum, Input, Context, Output, TL>::value;

// template <typename Definition, typename State, typename Policy>
// concept ValidPolicy = is_in_list<State, typename Definition::States>::value
// &&
//                       requires(const State &currentState,
//                                const Definition::Transitions &candidates) {
//                         {
//                           Policy::RunOne(currentState, candidates)
//                         } -> std::same_as<State>;
//                       };
//
// template <typename Definition, typename State> struct FirstValidPolicy {
//   static State RunOne(const State &currentState,
//                       const Definition::Transitions &candidates) {}
// };

template <typename Definition>
concept MachineDefinition =
    // Members
    requires {
      typename Definition::StateEnum;
      typename Definition::Input;
      typename Definition::Context;
      typename Definition::Output;

      typename Definition::States;
      typename Definition::Transitions;

      Definition::InitialState;
    } &&

    // Consistency Checks
    ValidStates<typename Definition::StateEnum, typename Definition::Context,
                typename Definition::States> &&

    ValidTransitions<typename Definition::StateEnum, typename Definition::Input,
                     typename Definition::Context, typename Definition::Output,
                     typename Definition::Transitions> &&

    requires {
      requires std::same_as<
          typename Definition::StateEnum,
          std::remove_cv_t<decltype(Definition::InitialState.Id)>>;
    };

template <typename TL> struct StepImpl; // Base case

template <typename... Transitions> struct StepImpl<TypeList<Transitions...>> {

  template <typename StateVariant, typename Input, typename Context,
            typename Output>
  static bool Run(StateVariant &currentState, const Input &input, Context &ctx,
                  Output &output) {

    return std::visit(
        [&](auto &activeState) -> bool {
          using Active = std::decay_t<decltype(activeState)>;
          bool matched = false;

          ([&]() -> void {
            if constexpr (std::is_same_v<typename Transitions::From, Active>) {
              if (!matched && Transitions::Guard(input, ctx)) {
                Transitions::Action(input, ctx, output);
                currentState = typename Transitions::To{};
                matched = true;
              }
            }
          } && ...);

          return matched;
        },
        currentState);
  }
};

template <typename TL, typename StateVariant, typename Input, typename Context,
          typename Output>
bool Step(StateVariant &currentState, const Input &input, Context &ctx,
          Output &output) {
  return StepImpl<TL>::Run(currentState, input, ctx, output);
}

template <
    typename Definition /*, typename Policy = FirstValidPolicy<Definition> */>
  requires MachineDefinition<Definition> // && ValidPolicy<Definition, Policy>
class Machine {
  using Input = typename Definition::Input;
  using Output = typename Definition::Output;
  using Context = typename Definition::Context;

  using States = typename Definition::States;
  using Transitions = typename Definition::Transitions;
  using VariantStates = VariantFromTypeList_t<States>;

public:
  Machine() : currentState(Definition::InitialState), ctx() {}

  bool Step(const Input &input, Output &output) {
    return Step<Transitions, VariantStates, Input, Context, Output>(
        currentState, input, ctx, output);
  }

private:
  VariantStates currentState;
  Context ctx;
  static constexpr States states = Definition::States;
  static constexpr Transitions transitions = Definition::Transitions;
};

} // namespace miniefsm
