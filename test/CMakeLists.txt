set(DISTRAC_TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/distrac_test_main.cpp
)

add_executable(distrac_test ${DISTRAC_TEST_SRCS})

set_property(TARGET distrac_test PROPERTY CXX_STANDARD 17)

target_link_libraries(distrac_test Catch2::Catch2)

target_link_libraries(distrac_test ${distrac_test_libs})

catch_discover_tests(distrac_test)
