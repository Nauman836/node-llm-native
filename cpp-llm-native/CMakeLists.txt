cmake_minimum_required(VERSION 3.20)

project(cpp_llm_native LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# turn off the native cpu build
set(GGML_NATIVE OFF CACHE BOOL "" FORCE)

# CPU backend only
set(GGML_CPU ON CACHE BOOL "" FORCE)
set(GGML_CUDA OFF CACHE BOOL "" FORCE)
set(GGML_VULKAN OFF CACHE BOOL "" FORCE)
set(GGML_METAL OFF CACHE BOOL "" FORCE)
set(GGML_HIP OFF CACHE BOOL "" FORCE)
set(GGML_SYCL OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/llama.cpp)

add_library(cpp_llm_native STATIC
    src/model.cpp
)

target_include_directories(cpp_llm_native
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(cpp_llm_native
PRIVATE
    llama
)

set_target_properties(cpp_llm_native PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)