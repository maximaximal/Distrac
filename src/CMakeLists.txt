set(DISTRAC_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/distrac.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/types.c
    PARENT_SCOPE)

set(DISTRAC_TEST_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/test_types.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/test_distrac.cpp
    PARENT_SCOPE)

set(DISTRAC_BIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/bin/main.cpp
    PARENT_SCOPE)

set(DISTRAC_CODEGEN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/event_definition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/property_definition.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/event.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen/property.cpp
    PARENT_SCOPE)

set(DISTRAC_CODEGEN_BIN_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/codegen-bin/codegen_main.cpp
    PARENT_SCOPE)
