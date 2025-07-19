# Project Win32 
cmake_minimum_required (VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

set(TargetName Win32)
if(NOT TARGET ${TargetName})
	file(GLOB_RECURSE sourceFiles "./Source/*.h" "./Source/*.cpp")
	add_library  (${TargetName} ${sourceFiles} )
	target_link_libraries(${TargetName} dbghelp.lib shcore.lib)
endif()
