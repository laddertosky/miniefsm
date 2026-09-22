#include "miniefsm.hpp"
#include "typelist.hpp"
#include <catch2/catch_test_macros.hpp>
#include <type_traits>
#include <variant>

using miniefsm::HasPriority;
using miniefsm::TransitionPriority_v;
using miniefsm::TypeList;
using miniefsm::TypeListSize_v;
using miniefsm::VariantFromTypeList_t;

namespace {

struct A {};
struct B {};
struct C {};

// ---- VariantFromTypeList_t (declared in typelist.hpp, global namespace) ----
static_assert(
    std::is_same_v<VariantFromTypeList_t<TypeList<A, B>>, std::variant<A, B>>);
static_assert(std::is_same_v<VariantFromTypeList_t<TypeList<A, B, C>>,
                             std::variant<A, B, C>>);

// ---- TypeListSize_v ----
static_assert(TypeListSize_v<TypeList<A, B>> == 2);
static_assert(TypeListSize_v<TypeList<A, B, C>> == 3);
static_assert(TypeListSize_v<TypeList<>> == 0);

// ---- TransitionPriority_v: defaults to 0 when Priority is absent ----
struct NoPriority {};
static_assert(TransitionPriority_v<NoPriority> == 0);

// ---- TransitionPriority_v: picks up an explicit Priority when present ----
struct WithPriority {
  static constexpr int Priority = 7;
};
static_assert(TransitionPriority_v<WithPriority> == 7);

// ---- HasPriority concept directly ----
static_assert(!HasPriority<NoPriority>);
static_assert(HasPriority<WithPriority>);

} // namespace

TEST_CASE("traits_test.cpp compiled, so all static_asserts passed") {
  REQUIRE(true);
}
