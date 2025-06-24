#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

#include "../../src/v5/ecs.hpp"

struct Position {
    float x, y;
};

struct Circle {
    float radius;
};

struct Color {
    unsigned char r, g, b, a;
};

struct MyECSConfig {
    using ComponentList = std::tuple<Position, Circle, Color>;
};

using MyECS = ecs::ComponentManager<MyECSConfig>;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        std::cerr << glfwGetError(0);
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui + GLFW", nullptr, nullptr);
    if (!window) {
        std::cerr << glfwGetError(0);
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // VSync

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    double lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    ecs::World<MyECS> world;

    world.createEntity<Position, Circle, Color>(Position{100.0f, 100.0f}, Circle{10.0f},
                                                Color{255, 0, 0, 255});
    world.createEntity<Position, Circle, Color>(Position{200.0f, 100.0f}, Circle{10.0f},
                                                Color{0, 255, 0, 255});
    world.createEntity<Position, Circle, Color>(Position{300.0f, 100.0f}, Circle{10.0f},
                                                Color{0, 0, 255, 255});
    world.createEntity<Position, Circle, Color>(Position{400.0f, 100.0f}, Circle{10.0f},
                                                Color{255, 0, 255, 255});

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Beispiel-GUI
        ImGui::Begin("Demo");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        world.forEach<Position, Circle, Color>([](Position& pos, Circle& circle, Color& color) {
            ImGui::GetBackgroundDrawList()->AddCircleFilled(
                ImVec2(pos.x, pos.y), circle.radius, IM_COL32(color.r, color.g, color.b, color.a),
                10);
        });

        world.forEach<Position, Circle>(
            [&](Position& pos, Circle&) { pos.x += 50.0f * deltaTime; });

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}