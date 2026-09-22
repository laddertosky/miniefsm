#pragma once

#include "typelist.hpp"
#include <concepts>

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

template <typename StateEnum, typename Context, typename TL>
struct ValidStateImpl; // Base case
//
template <typename StateEnum, typename Context, typename... States>
struct ValidStateImpl<StateEnum, Context, TypeList<States...>>
    : std::bool_constant<(StateDefinition<States, StateEnum, Context> && ...)> {
};

template <typename StateEnum, typename Context, typename TL>
concept ValidStates = ValidStateImpl<StateEnum, Context, TL>::value;

}; // namespace miniefsm
