add_executable(tl-test-wip)
target_sources(tl-test-wip PRIVATE wip.cpp)
target_link_libraries(tl-test-wip PRIVATE toylang::toylang)
