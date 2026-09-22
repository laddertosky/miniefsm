include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(miniefsm_tests
  concepts_test.cpp
  traits_test.cpp
  policy_test.cpp
  # machine_test.cpp
)
target_link_libraries(miniefsm_tests PRIVATE miniefsm Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(miniefsm_tests)
