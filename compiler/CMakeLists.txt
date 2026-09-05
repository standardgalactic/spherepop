cmake_minimum_required(VERSION 3.16)
project(spherepop C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(SP_ENABLE_SANITIZERS "Build with address/UB sanitizers" OFF)
option(SP_ENABLE_TESTS      "Build test suite" ON)

# Source files
set(SP_RUNTIME_SRCS
    src/runtime/bubble.c
    src/runtime/history.c
    src/runtime/constraints.c
    src/runtime/provenance.c
    src/runtime/scope.c
    src/runtime/value.c
    src/runtime/evaluator.c
    src/runtime/admissibility.c
    src/runtime/collapse.c
    src/runtime/action.c
    src/runtime/manifold.c
    src/runtime/topology.c
    src/runtime/reduction.c
    src/runtime/environment.c
    src/runtime/memory.c
    src/runtime/gc.c
    src/runtime/trajectories.c
)

set(SP_LEXER_SRCS
    src/lexer/lexer.c
    src/lexer/token.c
)

set(SP_PARSER_SRCS
    src/parser/parser.c
    src/parser/ast.c
    # NOTE: this list previously also listed src/parser/precedence.c and
    # src/parser/prettyprint.c, neither of which exists in the source tree
    # (confirmed: no file, no caller anywhere referencing functions from
    # them). Precedence climbing is implemented inline in parser.c
    # (parse_unary/parse_power/parse_mul/... functions), and AST pretty-
    # printing is implemented as node_print() in ast.c. Removed so the
    # project actually builds; re-add only if real, non-duplicate code is
    # split into those files.
)

set(SP_VM_SRCS
    src/vm/vm.c
    src/vm/bytecode.c
    src/vm/optimizer.c
    src/vm/scheduler.c
)

set(SP_GEOMETRY_SRCS
    src/geometry/regions.c
    src/geometry/manifolds.c
    src/geometry/curvature.c
    src/geometry/boundaries.c
    src/geometry/embeddings.c
    src/geometry/graph_layout.c
)

set(SP_SEMANTICS_SRCS
    src/semantics/semantic_fields.c
    src/semantics/contextual_equivalence.c
    src/semantics/category_semantics.c
    src/semantics/sheaf_semantics.c
    src/semantics/projection.c
    src/semantics/observerhood.c
)

set(SP_ALL_SRCS
    src/main.c
    src/repl.c
    src/config.c
    ${SP_RUNTIME_SRCS}
    ${SP_LEXER_SRCS}
    ${SP_PARSER_SRCS}
)

# Core library (for linking into tools and tests)
add_library(spherepop_core STATIC
    ${SP_RUNTIME_SRCS}
    ${SP_LEXER_SRCS}
    ${SP_PARSER_SRCS}
    src/config.c
    src/repl.c
)
target_include_directories(spherepop_core PUBLIC src)
target_link_libraries(spherepop_core m)

# Main interpreter executable
add_executable(spherepop src/main.c)
target_link_libraries(spherepop spherepop_core)
target_include_directories(spherepop PRIVATE src)

# Flat fixture conformance runner (experiments/flat/): a standalone C
# port of the canonical Pop/Refuse/Bind/Collapse event-log model, kept
# deliberately independent of spherepop_core -- see
# tools/fixtures/kernel.h for why. Not linked against spherepop_core.
add_executable(sp_fixtures
    tools/fixtures/main.c
    tools/fixtures/json.c
    tools/fixtures/kernel.c
    tools/fixtures/collapse_rules.c
    tools/fixtures/sugar.c
)
target_include_directories(sp_fixtures PRIVATE tools/fixtures)
target_compile_definitions(sp_fixtures PRIVATE
    FIXTURES_DEFAULT_DIR="${CMAKE_SOURCE_DIR}/../experiments/flat/fixtures")
target_link_libraries(sp_fixtures m)

# Compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(spherepop_core)
set_project_warnings(spherepop)

# Sanitizers
if(SP_ENABLE_SANITIZERS)
    include(cmake/Sanitizers.cmake)
    enable_sanitizers(spherepop_core)
endif()

# Tests
if(SP_ENABLE_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
install(TARGETS spherepop DESTINATION bin)
install(DIRECTORY src/stdlib/ DESTINATION share/spherepop/stdlib
        FILES_MATCHING PATTERN "*.sp")

# pkg-config
configure_file(cmake/spherepop.pc.in spherepop.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/spherepop.pc DESTINATION lib/pkgconfig)
