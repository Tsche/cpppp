target_sources(cpppp_lib PRIVATE parser.cpp interpreter.cpp lexemes.cpp token.cpp)
target_include_directories(cpppp_lib PUBLIC ${CMAKE_CURRENT_LIST_DIR})
