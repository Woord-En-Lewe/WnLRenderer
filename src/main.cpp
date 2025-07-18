#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <glad/gles2.h>
#include <wnlrenderer/renderer.h>
#include <window.h>

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <span>
#include <thread>
#include <vector>

#include "frag.h"
#include "vert.h"

void error_callback(int error, const char* description) {
    fmt::println(stderr, "GLFW Error: {}", description);
}

// NOLINTBEGIN
constexpr std::array<renderer::Vertex const, 4> vertices{
    // Positions from -0.5 to 0.5       // UV Coords
    renderer::Vertex{ {-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}}, // Top-left
    renderer::Vertex{{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
    renderer::Vertex{  {0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}, // Top-right
    renderer::Vertex{ {0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}}  // Bottom-right
};
// NOLINTEND

constexpr std::array<uint32_t, 6> indices{
    0, 1, 2,  // First Triangle
    2, 1, 3   // Second Triangle
};

int window_width           = 800;
int window_height          = 600;
glm::mat4 projectionMatrix = glm::ortho(0.0F,
                                        static_cast<float>(window_width),
                                        static_cast<float>(window_height),
                                        0.0F);

int main() {
    glfwSetErrorCallback(error_callback);
    window::GLFWContext _{};

    window::Window window{
        {.width = window_width, .height = window_height},
        "Window 1"
    };

    {
        auto context = window.get_context();
        gladLoadGLES2(glfwGetProcAddress);
    }

    std::jthread t{[&](const std::stop_token& stop_token) {
        auto context = window.get_context();
        renderer::ShaderProgram shaderProgram{std::string{vertex_shader},
                                              std::string{fragment_shader}};
        glfwSwapInterval(1);

        auto quad = std::make_shared<renderer::Mesh>(std::span{vertices},
                                                     std::span{indices});

        std::vector<uint32_t> textureData;
        textureData.resize(static_cast<size_t>(800 * 600));
        std::ranges::fill(textureData, 0U | (255U << 24) | (128U << 16) |
                                           (255U << 8) | (128U << 0));
        auto tex = std::make_shared<renderer::Texture<GL_RGBA>>(800, 600);
        tex->copy_data(std::as_bytes(std::span{textureData}));

        renderer::Renderable square{quad, tex};
        auto w = 200.0F;
        auto h = 100.0F;
        square.set_scale({w, h});

        glClearColor(0xFF / 255.0F, 0x0 / 255.0F, 0xFF / 255.0F, 1.0F);
        while (!stop_token.stop_requested()) {
            square.set_position(
                renderer::PositionCenter,
                {static_cast<float>(window_width) / 2.0F,
                 static_cast<float>(window_height) / 2.0F, 0.0F});

            glViewport(0, 0, window_width, window_height);
            glClear(GL_COLOR_BUFFER_BIT);
            shaderProgram.use();
            shaderProgram.set_mat4("u_Projection", projectionMatrix);

            square.draw(shaderProgram);

            window.swap_buffers();
        }
    }};

    while (window.should_close()) {
        glfwWaitEvents();
    }

    return 0;
}
