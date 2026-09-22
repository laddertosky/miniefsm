#pragma once

#include "typelist.hpp"
#include <concepts>

namespace miniefsm {

// A valid Transition must provide:
//   From, To         (required) - state types
//   Guard, Action    (required) - see signatures below
//   Priority         (optional) - static constexpr int; used by PriorityPolicy
//                       to break ties when multiple guards pass simultaneously.
//                       Defaults to 0 if omitted. See TransitionPriority<T>.
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

template <typename Transition> struct TransitionPriority {
  static constexpr int value = 0; // default when Transition has no ::Priority
};

template <typename Transition>
  requires requires {
    { Transition::Priority } -> std::convertible_to<int>;
  }
struct TransitionPriority<Transition> {
  static constexpr int value = Transition::Priority;
};

template <typename Transition>
inline constexpr int TransitionPriority_v =
    TransitionPriority<Transition>::value;
}; // namespace miniefsm
