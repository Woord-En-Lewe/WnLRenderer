#pragma once

#include <fmt/format.h>
#include <glad/gles2.h>

#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <span>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {
inline auto LoadShader(GLenum type, std::string const& shaderSrc) -> GLuint {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        throw std::runtime_error("failed to allocate shader");
    }
    auto const* data = shaderSrc.data();
    glShaderSource(shader, 1, &data, nullptr);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (compiled == 0) {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 1) {
            std::string infoLog;
            infoLog.resize(infoLen);
            glGetShaderInfoLog(shader, infoLen, nullptr, infoLog.data());
            glDeleteShader(shader);
            throw std::runtime_error(fmt::format(
                "Error compiling shader {}\n:{}",
                type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader",
                infoLog));
        }
        glDeleteShader(shader);
        throw std::runtime_error("Error compiling shader");
    }
    return shader;
}
};  // namespace

namespace renderer {

struct ShaderProgram {
    ShaderProgram(const std::string& vertex_shader,
                  const std::string& fragment_shader) {
        GLuint vertexShader   = LoadShader(GL_VERTEX_SHADER, vertex_shader);
        GLuint fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragment_shader);

        shaderProgram_ = glCreateProgram();
        if (shaderProgram_ == 0) {
            throw std::runtime_error("Failed to allocate shader program");
        }
        glAttachShader(shaderProgram_, vertexShader);
        glAttachShader(shaderProgram_, fragmentShader);
        glLinkProgram(shaderProgram_);

        glDetachShader(shaderProgram_, vertexShader);
        glDetachShader(shaderProgram_, fragmentShader);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        GLint linked;
        glGetProgramiv(shaderProgram_, GL_LINK_STATUS, &linked);
        if (linked == 0) {
            GLint infoLen = 0;
            glGetProgramiv(shaderProgram_, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                std::string infoLog;
                infoLog.resize(infoLen);
                glGetProgramInfoLog(shaderProgram_, infoLen, nullptr,
                                    infoLog.data());
                glDeleteProgram(shaderProgram_);
                throw std::runtime_error(
                    fmt::format("Error linking shader program:\n:{}", infoLog));
            }
            glDeleteProgram(shaderProgram_);
            throw std::runtime_error("Error linking shader program");
        }
    }

    ShaderProgram(ShaderProgram const&)            = delete;
    ShaderProgram& operator=(ShaderProgram const&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept
        : shaderProgram_(std::exchange(other.shaderProgram_, 0)) {
    }

    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        std::ranges::swap(other.shaderProgram_, shaderProgram_);
        return *this;
    }

    ~ShaderProgram() {
        glDeleteProgram(shaderProgram_);
    }

    auto use() const -> void {
        glUseProgram(shaderProgram_);
    }

    auto set_int(std::string const& name, int value) const -> void {
        glUniform1i(get_uniform_location(name), value);
    }

    auto set_int(char const* name, int value) const -> void {
        glUniform1i(get_uniform_location(name), value);
    }

    auto set_mat4(std::string const& name, glm::mat4 value) const -> void {
        glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE,
                           glm::value_ptr(value));
    }

    auto set_mat4(char const* name, glm::mat4 value) const -> void {
        glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE,
                           glm::value_ptr(value));
    }

    GLint get_attrib_location(std::string const& name) const {
        if (m_AttributeLocationCache.contains(name)) {
            return m_AttributeLocationCache.at(name);
        }
        GLint location = glGetAttribLocation(shaderProgram_, name.c_str());
        m_AttributeLocationCache[name] = location;
        return location;
    }

private:
    GLint get_uniform_location(std::string const& name) const {
        if (m_UniformLocationCache.contains(name)) {
            return m_UniformLocationCache.at(name);
        }
        GLint location = glGetUniformLocation(shaderProgram_, name.c_str());
        m_UniformLocationCache[name] = location;
        return location;
    }

private:
    GLuint shaderProgram_;
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
    mutable std::unordered_map<std::string, GLint> m_AttributeLocationCache;
};

struct VertexBuffer {  // TODO: Enable Shared from this
    VertexBuffer() {
        glGenBuffers(1, &vbo_);
    }

    ~VertexBuffer() {
        glDeleteBuffers(1, &vbo_);
    }

    VertexBuffer(VertexBuffer const&)            = delete;
    VertexBuffer& operator=(VertexBuffer const&) = delete;

    VertexBuffer(VertexBuffer&& other) noexcept
        : vbo_(std::exchange(other.vbo_, 0)) {
    }

    VertexBuffer& operator=(VertexBuffer&& other) noexcept {
        std::ranges::swap(other.vbo_, vbo_);
        return *this;
    }

    auto bind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    }
    auto unbind() const -> void {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    auto set_data(std::span<std::byte const> data) const -> void {
        bind();
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(data.size()),
                     data.data(), GL_STATIC_DRAW);
    }

private:
    GLuint vbo_;
};

struct IndexBuffer {  // TODO: Enable Shared from this
    IndexBuffer() {
        glGenBuffers(1, &ibo_);
    }

    ~IndexBuffer() {
        glDeleteBuffers(1, &ibo_);
    }

    IndexBuffer(IndexBuffer const&)            = delete;
    IndexBuffer& operator=(IndexBuffer const&) = delete;

    IndexBuffer(IndexBuffer&& other) noexcept
        : ibo_(std::exchange(other.ibo_, 0)) {
    }

    IndexBuffer& operator=(IndexBuffer&& other) noexcept {
        std::ranges::swap(other.ibo_, ibo_);
        return *this;
    }

    auto bind() const -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
    }
    auto unbind() const -> void {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    auto set_data(std::span<uint32_t const> data) -> void {
        bind();
        count_ = data.size();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(data.size_bytes()), data.data(),
                     GL_STATIC_DRAW);
    }

    auto get_count() const noexcept -> uint32_t {
        return count_;
    }

private:
    GLuint ibo_;
    uint32_t count_;
};

struct VertexAttribElement {
    unsigned int type;
    size_t count;
    unsigned char normalized;
    size_t offset;
};

struct VertexBufferLayout {
    template <typename T>
    auto push(size_t count, size_t offset) {
        static_assert(false, "Unknown type pushed to VertexBufferLayout");
    }

    template <>
    auto push<float>(size_t count, size_t offset) {
        elements_.push_back({GL_FLOAT, count, GL_FALSE, offset});
    }

    auto get_elements() const -> std::span<VertexAttribElement const> {
        return elements_;
    }

    auto get_stride() const -> size_t {
        return stride_;
    }

    auto set_stride(size_t stride) -> void {
        stride_ = stride;
    }

private:
    std::vector<VertexAttribElement> elements_;
    size_t stride_{};
};

struct VertexArray {
    VertexArray(std::shared_ptr<VertexBuffer> vbo,
                VertexBufferLayout const& layout)
        : vbo_(std::move(vbo)) {
        glGenVertexArrays(1, &vao_);
        bind();
        vbo_->bind();

        auto const& elements = layout.get_elements();
        size_t offset        = 0;

        for (auto i = 0; i < elements.size(); i++) {
            auto const& element = elements[i];
            glEnableVertexAttribArray(i);
            glVertexAttribPointer(
                i, static_cast<GLint>(element.count), element.type,
                element.normalized, static_cast<GLsizei>(layout.get_stride()),
                reinterpret_cast<void const*>(element.offset));  // NOLINT
        }
        unbind();
    }

    ~VertexArray() {
        glDeleteVertexArrays(1, &vao_);
    }
    VertexArray(VertexArray const&)            = delete;
    VertexArray& operator=(VertexArray const&) = delete;

    VertexArray(VertexArray&& other) noexcept
        : vao_(std::exchange(other.vao_, 0)) {
    }

    VertexArray& operator=(VertexArray&& other) noexcept {
        std::ranges::swap(other.vao_, vao_);
        return *this;
    }

    auto bind() const -> void {
        glBindVertexArray(vao_);
    }

    auto unbind() const -> void {
        glBindVertexArray(0);
    }

private:
    std::shared_ptr<VertexBuffer> vbo_;
    GLuint vao_;
};

template <int format =
              GL_RGBA>  // TODO: Unsized formats only for now and GL_R8/GL_RED
    requires(format == GL_RGBA || format == GL_RGB || format == GL_RED)
struct Texture {
    Texture(GLsizei width, GLsizei height)
        : width_(width), height_(height) {  // TODO: Stronger types
        if constexpr (format == GL_RGBA) {
            buffer_size_ = static_cast<size_t>(width_ * height_ * 4);
        } else if constexpr (format == GL_RGB) {
            buffer_size_ = static_cast<size_t>(width_ * height_ * 3);
        } else if constexpr (format == GL_RED) {
            buffer_size_ = static_cast<size_t>(width_ * height_ * 1);
        }
        glGenBuffers(1, &pbo_);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, buffer_size_, nullptr,
                     GL_STREAM_DRAW);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glGenTextures(1, &textureId_);
        bind();

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if constexpr (format == GL_RGBA || format == GL_RGB) {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                         GL_UNSIGNED_BYTE, nullptr);
        } else if constexpr (format == GL_RED) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, format,
                         GL_UNSIGNED_BYTE, nullptr);
        }
        unbind();
    }

    auto bind() -> void {
        glBindTexture(GL_TEXTURE_2D, textureId_);
    }
    auto unbind() -> void {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    auto copy_data(std::span<std::byte const> data) {
        bind();
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_);
        void* gpuMemory = glMapBufferRange(
            GL_PIXEL_UNPACK_BUFFER, 0, static_cast<GLsizeiptr>(data.size()),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (gpuMemory != nullptr) {
            std::memcpy(gpuMemory, data.data(), data.size());
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, format,
                        GL_UNSIGNED_BYTE, static_cast<void*>(nullptr));

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        unbind();
    }

    ~Texture() {
        glDeleteTextures(1, &textureId_);
        glDeleteBuffers(1, &pbo_);
    }
    Texture(Texture const&)            = delete;
    Texture& operator=(Texture const&) = delete;

    Texture(Texture&& other) noexcept
        : textureId_(std::exchange(other.textureId_, 0))
        , pbo_(std::exchange(other.pbo_, 0)) {
    }

    Texture& operator=(Texture&& other) noexcept {
        std::ranges::swap(other.textureId_, textureId_);
        std::ranges::swap(other.pbo_, pbo_);
        return *this;
    }

private:
    GLsizei width_;
    GLsizei height_;
    GLuint textureId_;
    GLuint pbo_;
    size_t buffer_size_;
};

template <typename T, typename M>
    requires std::is_standard_layout_v<T>
constexpr size_t checked_offset_of(M T::* member) {
    return reinterpret_cast<size_t>(&(static_cast<T const*>(nullptr)->*member));
}
struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;

    static renderer::VertexBufferLayout getLayout() {
        renderer::VertexBufferLayout layout;
        layout.set_stride(sizeof(Vertex));

        layout.push<float>(3, checked_offset_of(&Vertex::position));
        layout.push<float>(2, checked_offset_of(&Vertex::uv));

        return layout;
    }
};

struct Mesh {
    Mesh(std::span<Vertex const> vertices, std::span<uint32_t const> indices) {
        vertex_buffer_ = std::make_shared<renderer::VertexBuffer>();
        vertex_buffer_->set_data(std::as_bytes(vertices));

        index_buffer_ = std::make_shared<renderer::IndexBuffer>();
        index_buffer_->set_data(indices);

        vertex_array_ = std::make_unique<renderer::VertexArray>(
            vertex_buffer_, decltype(vertices)::value_type::getLayout());
    }

    auto draw() -> void {
        vertex_array_->bind();
        index_buffer_->bind();
        glDrawElements(GL_TRIANGLES,
                       static_cast<GLsizei>(index_buffer_->get_count()),
                       GL_UNSIGNED_INT, nullptr);
    }

private:
    std::unique_ptr<renderer::VertexArray> vertex_array_;
    std::shared_ptr<renderer::VertexBuffer> vertex_buffer_;
    std::shared_ptr<renderer::IndexBuffer> index_buffer_;
};

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

inline auto QuadMesh() -> Mesh {
    return {vertices, indices};
}

struct PositionTopLeft_t {};
struct PositionCenter_t {};

constexpr PositionTopLeft_t PositionTopLeft{};
constexpr PositionCenter_t PositionCenter{};

template <int format = GL_RGBA>
    requires(format == GL_RGBA || format == GL_RGB || format == GL_RED)
struct Renderable {
    Renderable(std::shared_ptr<Mesh> mesh,
               std::shared_ptr<Texture<format>> texture)
        requires(format != GL_RED)
        : mesh_(std::move(mesh))
        , texture_y(std::move(texture))
        , transform_(1.0F) {
    }

    Renderable(std::shared_ptr<Mesh> mesh,
               std::shared_ptr<Texture<format>> texture_y,
               std::shared_ptr<Texture<format>> texture_u,
               std::shared_ptr<Texture<format>> texture_v)
        requires(format == GL_RED)
        : mesh_(std::move(mesh))
        , texture_y(std::move(texture_y))
        , texture_u(std::move(texture_u))
        , texture_v(std::move(texture_v))
        , transform_(1.0F) {
    }

    void set_position(PositionCenter_t /*unused*/, glm::vec3 const& position) {
        position_ = position;

        dirty_ = true;
    }

    void set_position(PositionTopLeft_t /*unused*/, glm::vec3 const& position) {
        position_.x = position.x + (scale_.x / 2.0F);
        position_.y = position.y + (scale_.y / 2.0F);
        position_.z = position.z;

        dirty_ = true;
    }

    void set_scale(glm::vec2 scale) {
        scale_ = scale;
        dirty_ = true;
    }

    auto draw(ShaderProgram& shader) -> void
        requires(format != GL_RED)
    {
        if (dirty_) {
            glm::mat4 transMatrix = glm::translate(glm::mat4(1.0F), position_);
            glm::mat4 scaleMatrix = glm::scale(
                glm::mat4(1.0F), glm::vec3(scale_.x, scale_.y, 0.0F));

            transform_ = transMatrix * scaleMatrix;
        }
        glActiveTexture(GL_TEXTURE0);
        texture_y->bind();
        shader.set_int("u_texture", 0);
        shader.set_mat4("u_Transform", transform_);
        mesh_->draw();
    }

    auto draw(ShaderProgram& shader) -> void
        requires(format == GL_RED)
    {
        if (dirty_) {
            glm::mat4 transMatrix = glm::translate(glm::mat4(1.0F), position_);
            glm::mat4 scaleMatrix = glm::scale(
                glm::mat4(1.0F), glm::vec3(scale_.x, scale_.y, 0.0F));

            transform_ = transMatrix * scaleMatrix;
        }
        glActiveTexture(GL_TEXTURE0);
        texture_y->bind();
        glActiveTexture(GL_TEXTURE1);
        texture_u->bind();
        glActiveTexture(GL_TEXTURE2);
        texture_v->bind();

        shader.set_int("u_texture", 0);
        shader.set_mat4("u_Transform", transform_);
        mesh_->draw();
    }

    auto get_size() const
        -> std::pair<uint32_t, uint32_t> {  // TODO: Stronger types
        return std::make_pair(std::round(scale_.x), std::round(scale_.y));
    }

    auto get_position(PositionCenter_t /*unused*/) const
        -> std::pair<uint32_t, uint32_t> {  // TODO: Stronger types
        return std::make_pair(std::round(position_.x), std::round(position_.y));
    }

    auto get_position(PositionTopLeft_t /*unused*/) const
        -> std::pair<uint32_t, uint32_t> {  // TODO: Stronger types
        float top_left_x = position_.x - (scale_.x / 2.0F);
        float top_left_y = position_.y - (scale_.y / 2.0F);

        return std::make_pair(std::round(top_left_x), std::round(top_left_y));
    }

    auto get_texture() const -> std::shared_ptr<Texture<format>>
        requires(format != GL_RED)
    {
        return texture_y;
    }

    auto get_texture() const
        -> std::tuple<std::shared_ptr<Texture<format>>,  // TODO: Stronger types
                      std::shared_ptr<Texture<format>>,
                      std::shared_ptr<Texture<format>>>
        requires(format == GL_RED)
    {
        return std::make_tuple(texture_y, texture_u, texture_v);
    }

    auto get_format() const -> int {
        return format;
    }

private:
    std::shared_ptr<Mesh> mesh_;
    std::shared_ptr<Texture<format>> texture_y;
    std::shared_ptr<Texture<format>> texture_u;
    std::shared_ptr<Texture<format>> texture_v;

    glm::vec3 position_;
    glm::vec2 scale_;
    glm::mat4 transform_;

    bool dirty_{};
};

template <int format>
Renderable(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture<format>> texture)
    -> Renderable<format>;

};  // namespace renderer
