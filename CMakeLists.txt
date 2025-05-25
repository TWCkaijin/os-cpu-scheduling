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


add_subdirectory(pybind11)


file(GLOB MAIN_MODULE_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp") # 這行指定了主要的模組檔案，例如 main.cpp

# 收集 src/ 目錄中所有的 .cpp 檔案，這些檔案將一起編譯成一個模組
file(GLOB ALL_CXX_SOURCES_IN_SRC "${PROJECT_SOURCE_DIR}/src/*.cpp")

# 建立一個列表來存放所有的目標函式庫名稱
set(ALL_TARGETS "")


foreach(PRIMARY_SRC_FILE ${MAIN_MODULE_FILES}) # 迴圈處理 MAIN_MODULE_FILES 中的每個檔案 (目前只有 main.cpp)
    get_filename_component(MODULE_BASENAME ${PRIMARY_SRC_FILE} NAME_WE) # 從檔案路徑中獲取不含副檔名的檔案名，例如 "main"
    set(TARGET_MODULE_NAME "${MODULE_BASENAME}_cpp") # 設定目標模組的名稱，例如 "main_cpp"

    if(NOT ALL_CXX_SOURCES_IN_SRC)
        message(FATAL_ERROR "在 ${PROJECT_SOURCE_DIR}/src/ 中沒有找到 .cpp 檔案來建構模組 ${TARGET_MODULE_NAME}。")
    endif()

    # 使用所有在 src/ 目錄中找到的 .cpp 檔案來建立 pybind11 模組
    pybind11_add_module(${TARGET_MODULE_NAME} ${ALL_CXX_SOURCES_IN_SRC})
    
    set_target_properties(${TARGET_MODULE_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )
    
    install(TARGETS ${TARGET_MODULE_NAME}
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
    )

    # 將目標名稱加入列表
    list(APPEND ALL_TARGETS ${TARGET_MODULE_NAME})
endforeach()