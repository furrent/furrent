cmake_minimum_required (VERSION 3.22)
project (Furrent
    DESCRIPTION  "A tiny BitTorrent client made for furrents"
    HOMEPAGE_URL "github.com/furrent/furrent"
    LANGUAGES    CXX
)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(COMPILER_FLAGS -Wall -Wextra -Wpedantic)

# Setup executable
add_executable(furrent)
target_sources(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src/main.cpp
)
target_include_directories(furrent
    PRIVATE ${PROJECT_SOURCE_DIR}/src
)
target_compile_options(furrent
    PRIVATE ${COMPILER_FLAGS}
)

# Install binary
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
install(
    FILES       ${CMAKE_BINARY_DIR}/furrent
    DESTINATION ${PROJECT_SOURCE_DIR}/bin
)

# Documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/Doxyfile
        COMMENT "Generating API documentation with Doxygen"
    )
    install(
        FILES       ${CMAKE_BINARY_DIR}/doc
        DESTINATION ${PROJECT_SOURCE_DIR}
    )
endif()
