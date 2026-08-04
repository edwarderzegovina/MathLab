cmake_minimum_required(VERSION 3.16)
project(MathLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

# The library: the entities, the workspace that owns them, and every operation
# on them. No front-end, and deliberately no src/cli on its include path --
# core must not be able to see, let alone include, a user interface.
add_library(mathlab_core STATIC
        src/core/IObject.cpp
        src/core/MathEntity.cpp
        src/core/AlgebraEntity.cpp
        src/core/DataEntity.cpp
        src/core/Matrix.cpp
        src/core/Polynomial.cpp
        src/core/LinearSystem.cpp
        src/core/Dataset.cpp
        src/core/SolutionReport.cpp
        src/core/DatasetSummary.cpp
        src/core/Workspace.cpp
        src/core/MathLabException.cpp)
target_include_directories(mathlab_core PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/core")
target_compile_options(mathlab_core PRIVATE -Wall -Wextra)

# The terminal front-end: menus, prompts and printing, on top of the library.
add_library(mathlab_cli STATIC
        src/cli/ConsoleApp.cpp
        src/cli/ConsoleIO.cpp)
target_include_directories(mathlab_cli PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cli")
target_link_libraries(mathlab_cli PUBLIC mathlab_core)
target_compile_options(mathlab_cli PRIVATE -Wall -Wextra)

add_executable(proiect02 src/cli/main.cpp)
target_link_libraries(proiect02 PRIVATE mathlab_cli)
target_compile_options(proiect02 PRIVATE -Wall -Wextra)

# The desktop GUI (ImGui + GLFW), on top of the same mathlab_core library the
# CLI uses. Entirely optional: with MATHLAB_BUILD_GUI=OFF nothing in this
# block runs, so core/CLI configure and build with NO network access at all --
# the FetchContent calls for GLFW and ImGui are guarded by the option, not
# merely unused.
option(MATHLAB_BUILD_GUI "Build the ImGui/GLFW desktop GUI (mathlab-gui)" ON)

if (MATHLAB_BUILD_GUI)
    include(FetchContent)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE)
    FetchContent_Declare(imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.91.5
            GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glfw imgui)

    # Vendored ImGui sources: deliberately NOT built with -Wall -Wextra (that
    # requirement is for our own code only, not third-party code we didn't
    # write and can't fix).
    add_library(imgui STATIC
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
    target_include_directories(imgui PUBLIC
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends)
    target_link_libraries(imgui PUBLIC glfw)

    add_executable(mathlab-gui
            src/gui/main.cpp
            src/gui/GuiApp.cpp
            src/gui/panels/EntityList.cpp
            src/gui/panels/MatrixPanel.cpp
            src/gui/panels/PolynomialPanel.cpp
            src/gui/panels/SystemPanel.cpp
            src/gui/panels/DatasetPanel.cpp)
    target_include_directories(mathlab-gui PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/src/gui")
    target_link_libraries(mathlab-gui PRIVATE mathlab_core imgui glfw)
    target_compile_options(mathlab-gui PRIVATE -Wall -Wextra)
    if (APPLE)
        target_link_libraries(mathlab-gui PRIVATE "-framework OpenGL")
    endif()
endif()
