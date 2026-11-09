cmake_minimum_required(VERSION 3.16)
project(air_arcade_vm LANGUAGES CXX)

add_library(air_arcade_vm air_arcade_vm.cpp)
target_include_directories(air_arcade_vm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(air_arcade_vm PUBLIC cxx_std_17)
target_compile_options(air_arcade_vm PRIVATE -Wall -Wextra -Wpedantic)

add_executable(air_arcade_vm_test tests/vm_test.cpp)
target_link_libraries(air_arcade_vm_test PRIVATE air_arcade_vm)
enable_testing()
add_test(NAME air_arcade_vm_test COMMAND air_arcade_vm_test)
