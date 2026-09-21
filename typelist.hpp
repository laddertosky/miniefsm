#pragma once

#include <type_traits>
#include <variant>

template <typename... Types> struct TypeList {};

template <typename T> struct is_typelist : std::false_type {};

template <typename... Ts>
struct is_typelist<TypeList<Ts...>> : std::true_type {};

// Base case: type is not in an empty list
template <typename T, typename List> struct is_in_list : std::false_type {};

// Specialization: Check if T matches any of the types in the pack using fold
// expressions
template <typename T, typename... Us>
struct is_in_list<T, TypeList<Us...>>
    : std::bool_constant<(std::is_same_v<T, Us> || ...)> {};

template <typename TL> struct VariantFromTypeList; // Base case

template <typename... Types> struct VariantFromTypeList<TypeList<Types...>> {
  using type = std::variant<Types...>;
};

template <typename TL>
using VariantFromTypeList_t = typename VariantFromTypeList<TL>::type;
