add_executable(translator main.cpp 
				   Lexer.cpp 
				   Parser.cpp 
				   Intermediate.cpp
				   Bucket.cpp
				   Addressing.cpp
				   Intelx86Gen.cpp
				   CoffGenerator.cpp
				   )
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_features(translator PUBLIC cxx_std_20)
endif()