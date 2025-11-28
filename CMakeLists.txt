cmake_minimum_required(VERSION 3.10)
project(SpaceCpp)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)

# Handle spaces in directory name by quoting
file(GLOB SOURCES "Space C++/*.cpp")
file(GLOB HEADERS "Space C++/*.h")

add_executable(SpaceCpp ${SOURCES} ${HEADERS})

target_include_directories(SpaceCpp PRIVATE "Space C++")

find_package(glfw3 CONFIG QUIET)
if(glfw3_FOUND)
    target_link_libraries(SpaceCpp PRIVATE glfw)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED glfw3)
    target_include_directories(SpaceCpp PRIVATE ${GLFW_INCLUDE_DIRS})
    target_link_libraries(SpaceCpp PRIVATE ${GLFW_LIBRARIES})
endif()

target_link_libraries(SpaceCpp PRIVATE OpenGL::GL)

if(UNIX)
    target_link_libraries(SpaceCpp PRIVATE m)
endif()
