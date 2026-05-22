# Spherepop has no external dependencies by default.
# Optional: link against readline for enhanced REPL experience.
find_library(READLINE_LIB readline)
if(READLINE_LIB)
    message(STATUS "Found readline: ${READLINE_LIB}")
    target_link_libraries(spherepop PRIVATE ${READLINE_LIB})
    target_compile_definitions(spherepop PRIVATE SP_HAVE_READLINE=1)
endif()
