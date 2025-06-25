#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <random>
#include <string>
#include <type_traits>

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

struct Rectangle {
    float width, length;
};

struct Velocity {
    float dx, dy;
};

struct MyECSConfig {
    using ComponentList = std::tuple<Position, Circle, Color, Rectangle, Velocity>;
};

using MyECS = ecs::ComponentManager<MyECSConfig>;

template <typename CM, std::size_t I>
void ShowComponentEntry(ecs::EntityId id) {
    using T = typename CM::template ComponentType<I>;
    if constexpr (std::is_same_v<T, Position>) {
        ImGui::Text("Position");
    } else if constexpr (std::is_same_v<T, Velocity>) {
        ImGui::Text("Velocity");
    } else if constexpr (std::is_same_v<T, Circle>) {
        ImGui::Text("Circle");
    } else if constexpr (std::is_same_v<T, Rectangle>) {
        ImGui::Text("Rectangle");
    } else if constexpr (std::is_same_v<T, Color>) {
        ImGui::Text("Color");
    } else {
        ImGui::Text("Unknown component %zu", I);
    }
}

template <typename CM>
void ShowComponentsUI(ecs::EntityId id, std::size_t signature) {
    using ComponentList = typename CM::ComponentList;
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        (
            [&id, signature]<typename T>() {
                constexpr std::size_t CID = CM::template GetComponentID<T>();
                if ((signature & (1 << CID)) != 0) {
                    ShowComponentEntry<CM, CID>(id);
                }
            }.template operator()<std::tuple_element_t<Is, ComponentList>>(),
            ...);
    }(std::make_index_sequence<std::tuple_size_v<ComponentList>>{});
}

template <typename T>
T getRandom(T min, T max) {
    static std::mt19937 rng(std::random_device{}());

    if constexpr (std::is_floating_point_v<T>) {
        std::uniform_real_distribution<T> dist(min, max);
        return dist(rng);
    } else if constexpr (std::is_integral_v<T>) {
        using DistType = std::conditional_t<(sizeof(T) <= sizeof(int)), int, T>;
        std::uniform_int_distribution<DistType> dist(static_cast<DistType>(min),
                                                     static_cast<DistType>(max));
        return static_cast<T>(dist(rng));
    } else {
        static_assert(std::is_arithmetic_v<T>, "getRandom: unsupported type");
    }
}

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
    glfwSwapInterval(0);  // VSync

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
    world.createEntity<Position, Circle, Color, Velocity>(
        Position{400.0f, 100.0f}, Circle{10.0f}, Color{255, 0, 255, 255}, Velocity{50.0f, 20.0f});
    world.createEntity<Position, Rectangle, Color>(Position{300.0f, 200.0f},
                                                   Rectangle{10.0f, 20.0f}, Color{0, 0, 255, 255});
    size_t test = world.createEntity<Position, Rectangle, Color, Velocity>(
        Position{300.0f, 300.0f}, Rectangle{10.0f, 20.0f}, Color{0, 0, 255, 255},
        Velocity{70.0f, -20.0f});

    // fill space
    std::random_device rd;
    std::mt19937 rng(42);
    for (size_t i = 0; i < 100000; i++) {
        world.createEntity<Position, Circle, Color, Velocity>(
            Position{getRandom<float>(100.0f, 1000.0f), getRandom<float>(100.0f, 1000.0f)},
            Circle{10.0f},
            Color{getRandom<unsigned char>(0, 255), getRandom<unsigned char>(0, 255),
                  getRandom<unsigned char>(0, 255), 255},
            Velocity{getRandom<float>(-100.0f, +100.0f), getRandom<float>(-100.0f, +100.0f)});
    }

    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        // Beispiel-GUI
        ImGui::Begin("Demo");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("entities: %i", world.getEntityCount());
        ImGui::End();

        // update movement
        world.forEach<Position, Velocity>([&](Position& pos, Velocity& vel) {
            pos.x += vel.dx * deltaTime;
            pos.y += vel.dy * deltaTime;

            // borders
            if (pos.x < 0) {
                pos.x = 0;
                vel.dx = -vel.dx;
            }
            if (pos.x > display_w) {
                pos.x = display_w;
                vel.dx = -vel.dx;
            }
            if (pos.y < 0) {
                pos.y = 0;
                vel.dy = -vel.dy;
            }
            if (pos.y > display_h) {
                pos.y = display_h;
                vel.dy = -vel.dy;
            }
        });

        // draw circles
        world.forEach<Position, Circle, Color>([](Position& pos, Circle& circle, Color& color) {
            ImGui::GetBackgroundDrawList()->AddCircleFilled(
                ImVec2(pos.x, pos.y), circle.radius, IM_COL32(color.r, color.g, color.b, color.a),
                10);
        });

        // draw rectangle
        world.forEach<Position, Rectangle, Color>([](Position& pos, Rectangle& rect, Color& color) {
            ImGui::GetBackgroundDrawList()->AddRectFilled(
                ImVec2(pos.x, pos.y), ImVec2(pos.x + rect.length, pos.y + rect.width),
                IM_COL32(color.r, color.g, color.b, color.a));
        });

        // ImGui::Begin("Entities");
        // world.forEachEntity([&](ecs::EntityId id, ecs::detail::EntityLocation location) {
        //     // create tree node for entity
        //     if (ImGui::TreeNode(("Entity " + std::to_string(id)).c_str())) {
        //         auto signature = location.signature;
        //         ShowComponentsUI<MyECS>(id, signature);
        //         //  close tree node
        //         ImGui::TreePop();
        //     }
        // });
        // ImGui::End();

        ImGui::Render();

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