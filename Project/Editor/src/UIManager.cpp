#include "UIManager.h"
#include "Engine.h"
#include "WindowManager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

bool UIManager::initialized = false;

void UIManager::Initialize() {
    std::cout << "[Editor] UIManager initializing..." << std::endl;

    // Tell Engine to render to framebuffer
    Engine::SetRenderToFramebuffer(true);

    GLFWwindow* window = WindowManager::GetWindow();
    if (!window) {
        std::cout << "[Editor] No window!" << std::endl;
        return;
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cout << "[Editor] ImGui GLFW failed!" << std::endl;
        return;
    }

    if (!ImGui_ImplOpenGL3_Init("#version 430")) {
        std::cout << "[Editor] ImGui OpenGL failed!" << std::endl;
        return;
    }

    initialized = true;
    std::cout << "[Editor] UIManager ready!" << std::endl;
}

void UIManager::StartRender() {
    if (!initialized) return;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UIManager::Render() {
    if (!initialized) return;

    CreateDockSpace();
    ShowProperties();
    ShowConsole();
}

void UIManager::CreateDockSpace() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace", nullptr, flags);
    ImGui::PopStyleVar(3);

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                // Could set engine exit flag
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}

void UIManager::ShowProperties() {
    ImGui::Begin("Properties");
    ImGui::Text("Engine Architecture Test");
    ImGui::Separator();
    ImGui::BulletText("Engine: Proper separation");
    ImGui::BulletText("WindowManager: Handles GLFW");
    ImGui::BulletText("GraphicsManager: FBO only");
    ImGui::BulletText("SceneWindow: Displays FBO texture");
    ImGui::End();
}

void UIManager::ShowConsole() {
    ImGui::Begin("Console");
    ImGui::Text("Editor Console");
    ImGui::Separator();
    ImGui::Text("[Engine] Rendering to framebuffer");
    ImGui::Text("[Editor] Displaying in ImGui viewport");
    ImGui::Text("[Architecture] Clean separation achieved");
    ImGui::End();
}

void UIManager::EndRender() {
    if (!initialized) return;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
}

void UIManager::Shutdown() {
    if (!initialized) return;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    std::cout << "[Editor] UIManager shutdown" << std::endl;
}