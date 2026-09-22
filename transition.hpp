#pragma once

#include <concepts>

namespace miniefsm {

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
}; // namespace miniefsm
