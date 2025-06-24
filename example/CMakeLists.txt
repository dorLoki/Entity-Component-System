set(EXEC_NAME "${CMAKE_PROJECT_NAME}_gui")
include(FetchContent)

# ----------------------------------------
# Fetch GLFW
# ----------------------------------------
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# ----------------------------------------
# Fetch ImGui
# ----------------------------------------
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

# ImGui Backends manuell hinzufügen (GLFW + OpenGL3)
set(IMGUI_BACKEND_DIR ${imgui_SOURCE_DIR}/backends)
set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_glfw.cpp
    ${IMGUI_BACKEND_DIR}/imgui_impl_opengl3.cpp
)

# ----------------------------------------
# OpenGL
# ----------------------------------------
find_package(OpenGL REQUIRED)

# find all source files (only hpp and cpp)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp" "*.hpp")

add_executable(${EXEC_NAME} ${SOURCES} ${IMGUI_SRC})

target_include_directories(${EXEC_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${IMGUI_BACKEND_DIR}
)

target_link_libraries(${EXEC_NAME} PRIVATE glfw OpenGL::GL)