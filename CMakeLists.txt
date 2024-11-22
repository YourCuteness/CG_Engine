cmake_minimum_required(VERSION 3.5)
project(MyProject)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源文件路径
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(DLL_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 包含头文件路径
include_directories(${INCLUDE_DIR})

# 添加源文件
file(GLOB SOURCES "${SRC_DIR}/**/*.cpp" "${SRC_DIR}/*.cpp" "main.cpp")

# 添加可执行文件
add_executable(main ${SOURCES})

# 链接库
target_link_libraries(main ${LIB_DIR}/libglad.a ${LIB_DIR}/libglfw3dll.a)

# 确保生成控制台应用程序
if (WIN32)
    set_target_properties(main PROPERTIES LINK_FLAGS "-Wl,--subsystem,console")
endif()

# 复制DLL文件到构建目录
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DLL_DIR}/glfw3.dll"
    $<TARGET_FILE_DIR:main>)
