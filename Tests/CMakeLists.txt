cmake_minimum_required(VERSION 3.20)

project(stm32_fc_host_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

enable_testing()

add_library(unity STATIC
    vendor/unity/unity.c
)
target_include_directories(unity PUBLIC
    vendor/unity
)

add_library(fake_esc_output STATIC
    fakes/fake_esc_output.c
)
target_include_directories(fake_esc_output PUBLIC
    fakes
    ../HardwareLayer/Inc
)

add_library(fake_rc_receiver STATIC
    fakes/fake_rc_receiver.c
)
target_include_directories(fake_rc_receiver PUBLIC
    fakes
    ../Application/Inc
    ../HardwareLayer/Inc
)

add_executable(test_output
    Application/test_output.c
    ../Application/Src/output.c
)
target_include_directories(test_output PRIVATE
    ../Application/Inc
    ../HardwareLayer/Inc
    fakes
    vendor/unity
)
target_link_libraries(test_output PRIVATE
    unity
    fake_esc_output
)

add_test(NAME output COMMAND test_output)

add_executable(test_rc_input
    Application/test_rc_input.c
    ../Application/Src/rc-input.c
)
target_compile_definitions(test_rc_input PRIVATE
    STM32_FC_TEST
)
target_include_directories(test_rc_input PRIVATE
    ../Application/Inc
    ../HardwareLayer/Inc
    fakes
    vendor/unity
)
target_link_libraries(test_rc_input PRIVATE
    unity
    fake_rc_receiver
)

add_test(NAME rc_input COMMAND test_rc_input)
