// mathlab-gui entry point: GLFW window + ImGui context/backends + frame loop.
//
// THE macOS TRAP: without an explicit Core-profile context request, GLFW
// hands back a legacy 2.1 context on macOS and every ImGui shader fails to
// compile with "ERROR: 0:1: '' : version '150' is not supported". The four
// window hints below (major/minor/profile/forward-compat) and passing
// "#version 150" to ImGui_ImplOpenGL3_Init are both required.
#include <cstdio>

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#elif defined(_WIN32)
#include <windows.h>
#include <GL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GuiApp.h"
#include "Workspace.h"

namespace {

void glfwErrorCallback(int error, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

// Creates the window with the Core 3.2 hints (see the macOS trap note above).
GLFWwindow* createWindow() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::fprintf(stderr, "mathlab-gui: glfwInit() failed\n");
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(1280, 800, "MathLab", nullptr, nullptr);
    if (window == nullptr) {
        std::fprintf(stderr, "mathlab-gui: glfwCreateWindow() failed\n");
        glfwTerminate();
        return nullptr;
    }

    // A sensible floor so the two-pane layout (entity list + inspector) never
    // collapses into something unreadable; no maximum.
    glfwSetWindowSizeLimits(window, 900, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    return window;
}

bool initImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // Consistent spacing/rounding across every panel, applied once here
    // rather than at each individual ImGui:: call site.
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(10.0f, 10.0f);
    style.FramePadding = ImVec2(6.0f, 4.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.WindowRounding = 4.0f;
    style.FrameRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.ScrollbarSize = 14.0f;

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::fprintf(stderr, "mathlab-gui: ImGui_ImplGlfw_InitForOpenGL() failed\n");
        ImGui::DestroyContext();
        return false;
    }
    // "#version 150" matches the GLSL version implied by the 3.2 Core context
    // requested above; this is the other half of the macOS trap fix.
    if (!ImGui_ImplOpenGL3_Init("#version 150")) {
        std::fprintf(stderr, "mathlab-gui: ImGui_ImplOpenGL3_Init() failed\n");
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return false;
    }
    return true;
}

void shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void renderAndSwap(GLFWwindow* window, GuiApp& app, float deltaSeconds) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    app.renderFrame(deltaSeconds);

    ImGui::Render();
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    char title[64];
    std::snprintf(title, sizeof(title), "MathLab (%zu entities)", Workspace::instance().size());
    glfwSetWindowTitle(window, title);

    glfwSwapBuffers(window);
}

int runApp() {
    GLFWwindow* window = createWindow();
    if (window == nullptr) return 1;
    if (!initImGui(window)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    GuiApp app;
    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window) && !app.wantsQuit()) {
        glfwPollEvents();
        const double now = glfwGetTime();
        const float delta = static_cast<float>(now - lastTime);
        lastTime = now;

        renderAndSwap(window, app, delta);
    }

    shutdownImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

} // namespace

int main() {
    return runApp();
}
