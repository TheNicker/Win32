
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 17)
include_directories("./Win32/Include")
include_directories("./Win32/source")
include_directories(./External/LLUtils/Include)
add_subdirectory("Win32")
add_subdirectory("Example")