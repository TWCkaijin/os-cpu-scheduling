cmake_minimum_required(VERSION 3.10)
project(hh_core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: Release, Debug, RelWithDebInfo, MinSizeRel."
        FORCE)
endif()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/src/build")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")


add_subdirectory(submodule/pybind11)


file(GLOB MAIN_MODULE_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp")


file(GLOB ALL_CXX_SOURCES_IN_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")


set(ALL_TARGETS "")


foreach(PRIMARY_SRC_FILE ${MAIN_MODULE_FILES}) 
    get_filename_component(MODULE_BASENAME ${PRIMARY_SRC_FILE} NAME_WE) 
    set(TARGET_MODULE_NAME "${MODULE_BASENAME}_cpp")

    if(NOT ALL_CXX_SOURCES_IN_SRC)
        message(FATAL_ERROR "在 ${PROJECT_SOURCE_DIR}/src/ 中沒有找到 .cpp 檔案來建構模組 ${TARGET_MODULE_NAME}。")
    endif()

    pybind11_add_module(${TARGET_MODULE_NAME} ${ALL_CXX_SOURCES_IN_SRC})
    
    set_target_properties(${TARGET_MODULE_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    
    install(TARGETS ${TARGET_MODULE_NAME}
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    list(APPEND ALL_TARGETS ${TARGET_MODULE_NAME})
endforeach()