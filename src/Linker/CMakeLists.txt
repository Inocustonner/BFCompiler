add_executable(linker main.cpp
				   )
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_features(linker PUBLIC cxx_std_20)
endif()