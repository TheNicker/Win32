# Project GFXHelper 
cmake_minimum_required (VERSION 3.8)

set(TargetName Win32)

file(GLOB_RECURSE sourceFiles "./Source/*.h" "./Source/*.cpp")

add_library  (${TargetName} ${sourceFiles} )

target_link_libraries(${TargetName} dbghelp.lib)
