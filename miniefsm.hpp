#pragma once

#include "policy.hpp"
#include "state.hpp"
#include "transition.hpp"
#include "typelist.hpp"

#include <array>
#include <concepts>
#include <cstddef>
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

  template <typename Active, typename Transition, typename Input,
            typename Context>
  static void Evaluate(const Input &input, Context &ctx, bool &match,
                       int &priority) {
    if constexpr (std::is_same_v<typename Transition::From, Active>) {
      if (Transition::Guard(input, ctx)) {
        match = true;
        priority = TransitionPriority_v<Transition>;
      }
    }
  }

  template <typename Transition, typename StateVariant, typename Input,
            typename Context, typename Output>
  static void Fire(bool selected, StateVariant &currentState,
                   const Input &input, Context &ctx, Output &output) {
    if (selected) {
      Transition::Action(input, ctx, output);
      currentState = typename Transition::To{};
    }
  }

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
          ((Evaluate<Active, Transitions, Input, Context>(
                input, ctx, matches[index], priorities[index]),
            index++),
           ...);

          std::optional<size_t> selected;
          if constexpr (PriorityAwarePolicy<Policy, N>) {
            selected = Policy::Select(matches, priorities);
          } else {
            selected = Policy::Select(matches);
          }

          if (!selected.has_value())
            return false;

          // Fine, assume N won't be too large
          index = 0;
          (Fire<Transitions, StateVariant, Input, Context, Output>(
               selected.value() == index++, currentState, input, ctx, output),
           ...);
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
};

template <typename Definition, typename Policy = FirstMatchPolicy>
  requires MachineDefinition<Definition> &&
           ValidPolicy<Policy, TypeListSize_v<typename Definition::Transitions>>
class Machine {
  using Input = typename Definition::Input;
  using Output = typename Definition::Output;
  using Context = typename Definition::Context;

  using States = typename Definition::States;
  using Transitions = typename Definition::Transitions;
  using StateVariant = VariantFromTypeList_t<States>;

public:
  Machine() : currentState(Definition::InitialState), ctx() {}

  bool Step(const Input &input, Output &output) {
    return StepImpl<Transitions, Policy>::Run(currentState, input, ctx, output);
  }

  template <typename State> bool CurrentStateIs() const {
    return std::holds_alternative<State>(currentState);
  }

  const Context &GetContext() const { return ctx; }

private:
  StateVariant currentState;
  Context ctx;
};

} // namespace miniefsm
