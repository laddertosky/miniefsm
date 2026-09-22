#pragma once

#include "state.hpp"
#include "transition.hpp"
#include "typelist.hpp"

#include <concepts>
#include <type_traits>
#include <variant>

namespace miniefsm {

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
