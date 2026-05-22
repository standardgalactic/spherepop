function(set_project_warnings target)
    set(CLANG_WARNINGS
        -Wall -Wextra -Wpedantic
        -Wshadow -Wcast-align
        -Wno-unused-parameter
    )
    set(GCC_WARNINGS ${CLANG_WARNINGS} -Wlogical-op -Wduplicated-branches)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
        target_compile_options(${target} PRIVATE ${GCC_WARNINGS})
    endif()
endfunction()
