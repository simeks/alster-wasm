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
target_link_libraries(alster-wasm PUBLIC glfw)
target_link_options(alster-wasm PRIVATE
    -sUSE_WEBGPU
    -sUSE_GLFW=3
    -sASYNCIFY
    -sALLOW_MEMORY_GROWTH
    --js-library ${CMAKE_SOURCE_DIR}/src/lib.js
)
# Output emscripten html template
set_target_properties(alster-wasm PROPERTIES SUFFIX ".html")


