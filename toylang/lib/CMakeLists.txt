project(toylang-lib)

add_library(${PROJECT_NAME})
add_library(toylang::lib ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

target_sources(${PROJECT_NAME} PRIVATE
  include/toylang/diagnostic.hpp
  include/toylang/environment.hpp
  include/toylang/expr.hpp
  include/toylang/interpreter.hpp
  include/toylang/literal.hpp
  include/toylang/location.hpp
  include/toylang/media.hpp
  include/toylang/parser.hpp
  include/toylang/scanner.hpp
  include/toylang/source.hpp
  include/toylang/stmt.hpp
  include/toylang/token.hpp
  include/toylang/value.hpp

  include/toylang/util/buffer.hpp
  include/toylang/util/expr_str.hpp
  include/toylang/util/notifier.hpp
  include/toylang/util/reporter.hpp
  include/toylang/util.hpp

  src/internal/intrinsics.cpp
  src/internal/intrinsics.hpp

  src/util/expr_str.cpp
  src/util/notifier.cpp
  src/util/reporter.cpp

  src/environment.cpp
  src/expr.cpp
  src/interpreter.cpp
  src/media.cpp
  src/parser.cpp
  src/stmt.cpp
  src/util.cpp
  src/value.cpp
)

if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Werror=return-type -Wunused
  )
endif()
