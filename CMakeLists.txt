cmake_minimum_required(VERSION 3.20)

project(
    "alster"
    LANGUAGES CXX
)

project(alster-wasm)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(alster_sources
    src/main.cpp
)

add_executable(alster-wasm ${alster_sources})
add_custom_command(TARGET alster-wasm POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/index.html
	${CMAKE_BINARY_DIR}/index.html
)


