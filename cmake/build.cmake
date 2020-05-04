set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arc)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Handle resources for Qt targets
set(CMAKE_AUTORCC ON)

option(FORCE_COLORED_OUTPUT "Force compiler diagnostic colors" OFF)
if(FORCE_COLORED_OUTPUT)
    add_compile_options($<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>)
    add_compile_options($<$<CXX_COMPILER_ID:Clang>:-fcolor-diagnostics>)
endif()

if(WIN32 AND NOT CYGWIN)
    include(cmake/build_configs/win32.cmake)
else()
    include(cmake/build_configs/linux.cmake)
endif()
