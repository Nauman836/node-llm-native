cmake_minimum_required(VERSION 3.20)

project(node_llm_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Must be built through cmake-js
if(NOT DEFINED CMAKE_JS_INC)
    message(FATAL_ERROR "Use cmake-js to build this project.")
endif()

add_subdirectory(cpp-llm-native)

# node-addon-api include path
execute_process(
    COMMAND node -p "require('node-addon-api').include"
    OUTPUT_VARIABLE NODE_ADDON_API_DIR
)

string(REPLACE "\"" "" NODE_ADDON_API_DIR "${NODE_ADDON_API_DIR}")
string(STRIP "${NODE_ADDON_API_DIR}" NODE_ADDON_API_DIR)

add_library(node_llm_native SHARED
    addon/native.cpp
)

target_include_directories(node_llm_native PRIVATE
    ${CMAKE_JS_INC}
    ${NODE_ADDON_API_DIR}
    cpp-llm-native/include
)

target_compile_definitions(node_llm_native PRIVATE
    NAPI_CPP_EXCEPTIONS
    NODE_ADDON_API_CPP_EXCEPTIONS
)

target_link_libraries(node_llm_native PRIVATE
    cpp_llm_native
    ${CMAKE_JS_LIB}
)

set_target_properties(node_llm_native PROPERTIES
    PREFIX ""
    SUFFIX ".node"
)

# Windows
if(MSVC)
    target_compile_options(node_llm_native PRIVATE /EHsc)
endif()