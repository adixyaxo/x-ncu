cmake_minimum_required(VERSION 3.15)
project(X_NCU)

set(INCLUDE_PATHS 
    ${CMAKE_CURRENT_SOURCE_DIR}/boost_1_90_0 
    ${CMAKE_CURRENT_SOURCE_DIR}/Crow-1.3.1/include
)

add_executable(X_NCU main.cpp)

target_include_directories(X_NCU PUBLIC ${INCLUDE_PATHS})

# Tell Crow to look for <boost/asio.hpp> instead of <asio.hpp>
target_compile_definitions(X_NCU PUBLIC CROW_USE_BOOST)

find_package(Threads REQUIRED)
target_link_libraries(X_NCU PUBLIC Threads::Threads)

# Automatically copy the 'templates' and 'static' directories to the build folder after compiling
add_custom_command(TARGET X_NCU POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/templates
        ${CMAKE_CURRENT_BINARY_DIR}/templates
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/static
        ${CMAKE_CURRENT_BINARY_DIR}/static
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/database
        ${CMAKE_CURRENT_BINARY_DIR}/database
)