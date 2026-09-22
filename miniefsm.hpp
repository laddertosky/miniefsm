#pragma once

#include "policy.hpp"
#include "state.hpp"
#include "transition.hpp"
#include "typelist.hpp"

#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <optional>
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

template <typename TL, typename Policy> struct StepImpl; // Base case

template <typename... Transitions, typename Policy>
struct StepImpl<TypeList<Transitions...>, Policy> {

  template <typename StateVariant, typename Input, typename Context,
            typename Output>
  static bool Run(StateVariant &currentState, const Input &input, Context &ctx,
                  Output &output) {

    return std::visit(
        [&](auto &activeState) -> bool {
          using Active = std::decay_t<decltype(activeState)>;
          static constexpr std::size_t N = sizeof...(Transitions);
          std::array<bool, N> matches{};
          std::array<int, N> priorities{};

          size_t index = 0;
          ([&]() -> void {
            if constexpr (std::is_same_v<typename Transitions::From, Active>) {
              if (Transitions::Guard(input, ctx)) {
                matches[index] = true;
                priorities[index] = TransitionPriority_v<Transitions>;
              }
            }

            index++;
          } && ...);

          std::optional<size_t> selected;
          if constexpr (requires { Policy::Select(matches); }) {
            selected = Policy::Select(matches, priorities);
          } else {
            selected = Policy::Select(matches);
          }

          if (!selected.has_value())
            return false;

          // Fine, assume N won't be too large
          index = 0;
          ([&]() -> void {
            if (selected.value() == index) {
              Transitions::Action(input, ctx, output);
              currentState = typename Transitions::To{};
            }
            index++;
          } && ...);
          return true;
        },
        currentState);
  }
};

template <typename TL, typename Policy, typename StateVariant, typename Input,
          typename Context, typename Output>
bool Step(StateVariant &currentState, const Input &input, Context &ctx,
          Output &output) {
  return StepImpl<TL, Policy>::Run(currentState, input, ctx, output);
}

template <typename Definition, typename Policy = FirstMatchPolicy>
  requires MachineDefinition<Definition> &&
           ValidPolicy<Policy, TypeListSize_v<typename Definition::Transitions>>
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
    return Step<Transitions, Policy, VariantStates, Input, Context, Output>(
        currentState, input, ctx, output);
  }

private:
  VariantStates currentState;
  Context ctx;
  static constexpr States states = Definition::States;
  static constexpr Transitions transitions = Definition::Transitions;
};

} // namespace miniefsm
