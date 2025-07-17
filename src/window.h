#pragma once

#include <GLFW/glfw3.h>

#include <atomic>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <stdexcept>
#include <utility>

namespace window {

struct WindowSize {
    int width;
    int height;
};

// Precondition: Can only exist on the main thread
// Precondition: GLFWContext must be created before and be alive during
class Window {
public:
    explicit Window(WindowSize size, std::string window_title)
        : window_width_(size.width)
        , window_height_(size.height)
        , window_title_(std::move(window_title))
        , projection_matrix_(glm::ortho(0.0F,
                                        static_cast<float>(window_width_),
                                        static_cast<float>(window_height_),
                                        0.0F)) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

        GLFWwindow* window = glfwCreateWindow(window_width_, window_height_,
                                              "Window 1", nullptr, nullptr);
        if (window == nullptr) {
            char const* error = nullptr;
            glfwGetError(&error);
            throw std::runtime_error(error);
        }
        window_.reset(window);
        glfwSetWindowUserPointer(window_.get(), this);
        glfwSetWindowSizeCallback(window_.get(), framebuffer_size_callback);
        glfwSetKeyCallback(window_.get(), key_callback);
    }

    auto swap_buffers() -> void {
        glfwSwapBuffers(window_.get());
    }

    auto should_close() -> bool {
        return glfwWindowShouldClose(window_.get()) != GLFW_TRUE;
    }

    struct OpenGLContext {
        ~OpenGLContext() {
            glfwMakeContextCurrent(nullptr);
            window_.context_out_.store(false, std::memory_order_release);
        }

        OpenGLContext(OpenGLContext const&)            = delete;
        OpenGLContext& operator=(OpenGLContext const&) = delete;

        OpenGLContext(OpenGLContext&) noexcept             = delete;
        OpenGLContext& operator=(OpenGLContext&&) noexcept = delete;

    private:
        friend Window;
        OpenGLContext(Window& window) : window_(window) {};

        Window& window_;  // TODO: shared_ptr?
    };

    [[nodiscard]] auto get_context() -> OpenGLContext {
        bool expected = false;
        if (!context_out_.compare_exchange_strong(expected, true,
                                                  std::memory_order_acq_rel)) {
            throw std::runtime_error("OpenGL context already acquired!");
        }

        glfwMakeContextCurrent(window_.get());
        return {*this};
    }

private:
    static void framebuffer_size_callback(GLFWwindow* window,
                                          int width,
                                          int height) {
        auto* w = static_cast<Window*>(glfwGetWindowUserPointer(window));
        w->window_width_      = width;
        w->window_height_     = height;
        w->projection_matrix_ = glm::ortho(0.0F, static_cast<float>(width),
                                           static_cast<float>(height), 0.0F);
    }

    static void key_callback(
        GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

private:
    int window_width_;
    int window_height_;
    std::string window_title_;
    glm::mat4 projection_matrix_;
    std::unique_ptr<GLFWwindow, decltype(&glfwDestroyWindow)> window_{
        nullptr, &glfwDestroyWindow};
    std::atomic<bool> context_out_ = false;
};

struct GLFWContext {
    GLFWContext() {
        if (glfwInit() == GLFW_FALSE) {
            throw std::runtime_error("Failed to init GLFW");
        }
    }
    ~GLFWContext() {
        glfwTerminate();
    }

    GLFWContext(GLFWContext const&)            = delete;
    GLFWContext(GLFWContext&&)                 = delete;
    GLFWContext& operator=(GLFWContext const&) = delete;
    GLFWContext& operator=(GLFWContext&&)      = delete;
};

}  // namespace window
