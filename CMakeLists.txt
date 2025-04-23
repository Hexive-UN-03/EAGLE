cmake_minimum_required(VERSION 3.11)

project(EAGLE LANGUAGES CXX)

# Define an INTERFACE library (no source files)
add_library(EAGLE INTERFACE)

# Specify the include directory for the header file
target_include_directories(EAGLE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Optional: Provide an alias for the library, for namespacing
add_library(EAGLE::EAGLE ALIAS EAGLE)
