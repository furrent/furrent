cmake_minimum_required (VERSION 3.10)
project (Furrent
    DESCRIPTION  "A tiny BitTorrent client made for furrents"
    HOMEPAGE_URL "github.com/furrent/furrent"
    LANGUAGES    CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Undefined behavior checking at runtime
option(USAN     OFF)
# Compile with coverage support
option(COVERAGE OFF)

set(COMPILER_FLAGS
    -Wall -Wextra -Wpedantic
    $<$<BOOL:${USAN}>:-fsanitize=undefined -fno-omit-frame-pointer>
    $<$<BOOL:${COVERAGE}>:--coverage>
)

set(LINKER_FLAGS
    $<$<BOOL:${USAN}>:-fsanitize=undefined -fno-omit-frame-pointer>
    $<$<BOOL:${COVERAGE}>:--coverage>
)

# ================
# Setup executable
# ================
add_executable(furrent src/mt/thread_pool.hpp src/mt/thread_pool.inl)
target_sources(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src/main.cpp
)
target_include_directories(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)
target_compile_options(furrent
    PRIVATE ${COMPILER_FLAGS}
)
target_link_options(furrent
    PRIVATE ${LINKER_FLAGS}
)

# ======================
# Install binary to bin/
# ======================
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
install(
    FILES       ${CMAKE_BINARY_DIR}/furrent
    DESTINATION ${PROJECT_SOURCE_DIR}/bin
)

# =============
# Documentation
# =============
find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
        COMMENT "Generating API documentation with Doxygen"
    )
    install(
        FILES       ${CMAKE_BINARY_DIR}/doc
        DESTINATION ${PROJECT_SOURCE_DIR}
        # Skip if documentation hasn't been generated
        OPTIONAL
    )
endif()

# ========
# Coverage
# ========
if (COVERAGE)
    add_custom_target(coverage
        COMMAND gcovr
        --root ${PROJECT_SOURCE_DIR}
        -v
        --html --html-details coverage.html
        COMMENT "Generating code coverage"
    )
endif()

# ===========
# Build tests
# ===========
add_executable(furrent_test EXCLUDE_FROM_ALL)
target_sources(furrent_test
    # Every source file except src/main.cpp should appear here
    PRIVATE ${PROJECT_SOURCE_DIR}/test/entrypoint.cpp
    PRIVATE ${PROJECT_SOURCE_DIR}/third_party/catch2/catch.hpp
)
target_include_directories(furrent_test
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${PROJECT_SOURCE_DIR}/third_party/catch2
)
target_compile_options(furrent_test
    PRIVATE ${COMPILER_FLAGS}
)
target_link_options(furrent_test
    PRIVATE ${LINKER_FLAGS}
)

# =======
# Testing
# =======
enable_testing()
add_test(NAME tstandard
    COMMAND ${CMAKE_BINARY_DIR}/furrent_test
)
add_test(NAME tvalgrind
    COMMAND valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes
    --track-origins=yes
    ${CMAKE_BINARY_DIR}/furrent_test
    # This line disables the test by default.
    # If you want to run it, use `ctest -C valgrind`
    CONFIGURATIONS valgrind
)
