add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)

target_sources(${PROJECT_NAME} PRIVATE
  include/toylang/error_reporter.hpp
  include/toylang/expr.hpp
  include/toylang/location.hpp
  include/toylang/parser.hpp
  include/toylang/scanner.hpp
  include/toylang/token.hpp
  include/toylang/trivial.hpp

  src/error_reporter.cpp
  src/expr.cpp
  src/parser.cpp
)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type
  )
endif()
