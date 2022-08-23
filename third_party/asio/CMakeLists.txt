add_library(asio STATIC)
target_include_directories(asio PRIVATE include)
target_sources(asio PRIVATE src/asio.cpp)
target_compile_definitions(asio PRIVATE ASIO_SEPARATE_COMPILATION)
