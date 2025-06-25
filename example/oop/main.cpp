#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <random>
#include <string>
#include <type_traits>

struct Position {
    float x, y;
};

struct Color {
    unsigned char r, g, b, a;
};

struct Velocity {
    float dx, dy;
};

class Entity {
   protected:
    Position position;
    Velocity velocity;
    Color color;

   public:
    Entity(Position pos, Velocity vel, Color col) : position(pos), velocity(vel), color(col) {}

    virtual ~Entity() = default;

    virtual void move(float dt, int display_w, int display_h) {
        position.x += velocity.dx * dt;
        position.y += velocity.dy * dt;

        // borders
        if (position.x < 0) {
            position.x = 0;
            velocity.dx = -velocity.dx;
        }
        if (position.x > display_w) {
            position.x = display_w;
            velocity.dx = -velocity.dx;
        }
        if (position.y < 0) {
            position.y = 0;
            velocity.dy = -velocity.dy;
        }
        if (position.y > display_h) {
            position.y = display_h;
            velocity.dy = -velocity.dy;
        }
    }

    virtual void draw() const = 0;

    const Position& getPosition() const { return position; }
    const Velocity& getVelocity() const { return velocity; }
};

class CircleEntity : public Entity {
   private:
    float radius;

   public:
    CircleEntity(Position pos, Velocity vel, Color col, float rad)
        : Entity(pos, vel, col), radius(rad) {}

    void draw() const override {
        ImGui::GetBackgroundDrawList()->AddCircleFilled(
            ImVec2(position.x, position.y), radius, IM_COL32(color.r, color.g, color.b, color.a),
            10);
    }
};

class RectangleEntity : public Entity {
   private:
    float width, length;

   public:
    RectangleEntity(Position pos, Velocity vel, Color col, float wid, float len)
        : Entity(pos, vel, col), width(wid), length(len) {}

    void draw() const override {
        ImGui::GetBackgroundDrawList()->AddRectFilled(
            ImVec2(position.x, position.y), ImVec2(position.x + length, position.y + width),
            IM_COL32(color.r, color.g, color.b, color.a));
    }
};

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

    std::vector<Entity*> world;
    world.push_back(new RectangleEntity{Position{100.0f, 100.0f}, Velocity{70.0f, -20.0f},
                                        Color{0, 0, 255, 255}, 10.0f, 20.0f});

    // fill space
    std::random_device rd;
    std::mt19937 rng(42);
    for (size_t i = 0; i < 100000; i++) {
        world.push_back(new CircleEntity{
            Position{getRandom<float>(100.0f, 1000.0f)},
            Velocity{getRandom<float>(-100.0f, +100.0f), getRandom<float>(-100.0f, +100.0f)},
            Color{getRandom<unsigned char>(0, 255), getRandom<unsigned char>(0, 255),
                  getRandom<unsigned char>(0, 255), 255},
            10.0f});
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
        ImGui::Text("entities: %i", world.size());
        ImGui::End();

        for (auto entity : world) {
            entity->move(deltaTime, display_w, display_h);
        }

        for (auto entity : world) {
            entity->draw();
        }

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