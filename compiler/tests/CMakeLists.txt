# tests/CMakeLists.txt — unit/integration test executables.
#
# NOTE: this file did not exist even though the top-level CMakeLists.txt
# has always called add_subdirectory(tests) whenever SP_ENABLE_TESTS is ON
# (the default) -- meaning the project could not be configured at all with
# its own default options. Added so `cmake .. && make && ctest` works out
# of the box, matching what src/*.c and the existing test_*.c files already
# assume.
#
# Each test_*.c file is self-contained (its own main(), TEST() macro) and
# already returns 0/1 based on pass/fail, so each maps to one CTest case.

function(sp_add_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} spherepop_core)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

sp_add_test(test_admissibility admissibility/test_admissibility.c)
sp_add_test(test_programs      integration/test_programs.c)
sp_add_test(test_lexer         lexer/test_lexer.c)
sp_add_test(test_bubble        runtime/test_bubble.c)
sp_add_test(test_history       runtime/test_history.c)
