add_executable(bin main.cpp 
				   Lexer.cpp 
				   Parser.cpp 
				   Intermediate.cpp
				   Bucket.cpp
				   Addressing.cpp
				   #Intelx86Gen.cpp
				   )
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_features(bin PUBLIC cxx_std_20)
endif()