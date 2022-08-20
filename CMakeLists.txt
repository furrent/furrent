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

    # We need to force "char" to be undefined because bencoded strings can contain any unsigned byte value ranging from
    # 0 to 255
    -funsigned-char

    $<$<BOOL:${USAN}>:-fsanitize=undefined -fno-omit-frame-pointer>
    $<$<BOOL:${COVERAGE}>:--coverage>
)

set(COMPILER_DEFINITIONS FMT_HEADER_ONLY)

set(LINKER_FLAGS
    $<$<BOOL:${USAN}>:-fsanitize=undefined -fno-omit-frame-pointer>
    $<$<BOOL:${COVERAGE}>:--coverage>
)

# ===============
# SHA1 dependency
# ===============
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/smallsha1)

# ======================
# HTTP client dependency
# ======================
include(FetchContent)
FetchContent_Declare(cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400)
FetchContent_MakeAvailable(cpr)

# =================
# Find source files
# =================
file(GLOB SOURCES_LIB "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SOURCE "main.cpp")
file(GLOB SOURCES_TEST "${PROJECT_SOURCE_DIR}/test/*.cpp")

# =============
# Setup library
# =============
add_library(furrent_lib STATIC)
target_sources(furrent_lib PRIVATE ${SOURCES_LIB})
target_include_directories(furrent_lib
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${PROJECT_SOURCE_DIR}/third_party
)
target_compile_options(furrent_lib
    PRIVATE ${COMPILER_FLAGS}
)
target_link_options(furrent_lib
    PRIVATE ${LINKER_FLAGS}
)
target_compile_definitions(furrent_lib PRIVATE ${COMPILER_DEFINITIONS})
target_link_libraries(furrent_lib smallsha1)
target_link_libraries(furrent_lib cpr::cpr)

# =====================
# Setup main executable
# =====================
add_executable(furrent)
target_sources(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src/main.cpp
)
target_include_directories(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${PROJECT_SOURCE_DIR}/third_party
)
target_compile_options(furrent
    PRIVATE ${COMPILER_FLAGS}
)
target_link_options(furrent
    PRIVATE ${LINKER_FLAGS}
)
target_compile_definitions(furrent PRIVATE ${COMPILER_DEFINITIONS})
target_link_libraries(furrent furrent_lib)

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
target_sources(furrent_test PRIVATE ${SOURCES_TEST})
target_include_directories(furrent_test
    PRIVATE ${PROJECT_SOURCE_DIR}/src
    PRIVATE ${PROJECT_SOURCE_DIR}/test
    PRIVATE ${PROJECT_SOURCE_DIR}/third_party
)
target_compile_options(furrent_test
    PRIVATE ${COMPILER_FLAGS}
)
target_link_options(furrent_test
    PRIVATE ${LINKER_FLAGS}
)
target_compile_definitions(furrent_test PRIVATE ${COMPILER_DEFINITIONS})
target_link_libraries(furrent_test furrent_lib)

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