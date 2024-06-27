# CMakeList.txt : CMake project for GFXRendererD3D11, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if(NOT TARGET Win32Example)
	include_directories(../External/LLUtils/Include)
	# Add source to this project's executable.
	add_executable (Win32Example "Example.cpp")

	target_link_libraries(Win32Example Win32 shcore)

	if(MSVC)
	  #target_compile_options(Win32Example PRIVATE /W4 /WX)
	else()
	  target_compile_options(Win32Example PRIVATE -Wall -Wextra -pedantic -Werror)
	endif()
endif()