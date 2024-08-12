cmake_minimum_required(VERSION 3.25)
enable_language(CXX)
project(DEM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")



# libDEM
set(LIBDEM_SOURCES include/DEM.hpp include/Map.hpp include/Utility.hpp)
set(LIBDEM_INCLUDE_DIRECTORY include)


# libDEM.a / libDEM.lib
add_library("${PROJECT_NAME}_static" STATIC ${LIBDEM_SOURCES})
target_include_directories("${PROJECT_NAME}_static" PUBLIC ${LIBDEM_INCLUDE_DIRECTORY})
set_target_properties(
    "${PROJECT_NAME}_static"
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    POSITION_INDEPENDENT_CODE ON
    LINKER_LANGUAGE CXX
)


# libDEM.so / libDEM.dll
add_library("${PROJECT_NAME}_shared" SHARED ${LIBDEM_SOURCES})
target_include_directories("${PROJECT_NAME}_shared" PUBLIC ${LIBDEM_INCLUDE_DIRECTORY})
set_target_properties(
    "${PROJECT_NAME}_shared"
    PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    POSITION_INDEPENDENT_CODE ON
    LINKER_LANGUAGE CXX
)
