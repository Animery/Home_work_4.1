// #define DEBUG_more

#include "../include/engine.hpp"

#include <algorithm>
#include <array>
// #include <cassert>
#include <chrono>
#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <vector>

#include <SDL2/SDL.h>

#include "../include/glad/glad.h"
#include "../include/shader.hpp"

namespace my_engine
{
static void APIENTRY
callback_opengl_debug(GLenum                       source,
                      GLenum                       type,
                      GLuint                       id,
                      GLenum                       severity,
                      GLsizei                      length,
                      const GLchar*                message,
                      [[maybe_unused]] const void* userParam);

static std::array<std::string_view, 17> event_names = {
    { /// input events
      "left_pressed",
      "left_released",
      "right_pressed",
      "right_released",
      "up_pressed",
      "up_released",
      "down_pressed",
      "down_released",
      "select_pressed",
      "select_released",
      "start_pressed",
      "start_released",
      "button1_pressed",
      "button1_released",
      "button2_pressed",
      "button2_released",
      /// virtual console events
      "turn_off" }
};

std::ostream& operator<<(std::ostream& stream, const event e)
{
    uint32_t value     = static_cast<uint32_t>(e);
    uint32_t max_value = static_cast<uint32_t>(event::turn_off);
    if (value <= max_value)
    {
        stream << event_names[value];
        return stream; /* code */
    }
    else
    {
        throw std::runtime_error("to big event value");
    }
}

static std::ostream& operator<<(std::ostream& out, const SDL_version& v)
{
    out << static_cast<int>(v.major) << '.';
    out << static_cast<int>(v.minor) << '.';
    out << static_cast<int>(v.patch);
    return out;
}

struct bind
{
    SDL_Keycode      key;
    std::string_view name;
    event            event_pressed;
    event            event_released;
};

const std::array<bind, 8> keys{
    { { SDLK_w, "up", event::up_pressed, event::up_released },
      { SDLK_a, "left", event::left_pressed, event::left_released },
      { SDLK_s, "down", event::down_pressed, event::down_released },
      { SDLK_d, "right", event::right_pressed, event::right_released },
      { SDLK_LCTRL,
        "button1",
        event::button1_pressed,
        event::button1_released },
      { SDLK_SPACE,
        "button2",
        event::button2_pressed,
        event::button2_released },
      { SDLK_ESCAPE, "select", event::select_pressed, event::select_released },
      { SDLK_RETURN, "start", event::start_pressed, event::start_released } }
};

static bool check_input(const SDL_Event& e, const bind*& result)
{
    using namespace std;

    const auto it = find_if(begin(keys), end(keys), [&](const bind& b) {
        return b.key == e.key.keysym.sym;
    });

    if (it != end(keys))
    {
        result = &(*it);
        return true;
    }
    return false;
}

class engine_impl : public engine
{
public:
    std::string initialize(std::string_view /*config*/) final;
    bool        read_input(event& e) final;
    void        render_triangle(const triangle&) final;
    void        swap_buffers() final;
    void        uninitialize() final;

private:
    SDL_Window*   window      = nullptr;
    size_t width = 320;
    size_t height = 240;
    SDL_GLContext gl_context  = nullptr;
    GLuint        program_id_ = 0;

    GLuint vertexVBO;

    bool core_or_es = true;
};

std::string engine_impl::initialize(std::string_view /*config*/)

{
    std::stringstream serr;

    SDL_version compiled;
    SDL_version linked;

    SDL_VERSION(&compiled);
    SDL_GetVersion(&linked);

    if (SDL_COMPILEDVERSION !=
        SDL_VERSIONNUM(linked.major, linked.minor, linked.patch))
    {
        serr << "warning: SDL2 compiled and linked version mismatch: "
             << compiled << " " << linked << std::endl;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        const char* error_message = SDL_GetError();
        serr << "error: failed call SDL_Init: " << error_message;
        return serr.str();
    }

    if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG) !=
        0)
    {
        const char* error_message = SDL_GetError();
        serr << "error: failed call SDL_GL_SetAttribute: " << error_message;
        return serr.str();
    }

    window = SDL_CreateWindow("OpenGL",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width*8, // *12 = 3840
                              height*8, // *12 = 2880
                              SDL_WINDOW_OPENGL);
    if (window == nullptr)
    {
        const char* err_message = SDL_GetError();
        serr << "error: failed call SDL_CreateWindow: " << err_message
             << std::endl;
        SDL_Quit();
        return serr.str();
    }

    int gl_major_ver, gl_minor_ver, gl_context_profile;

    if (core_or_es)
    {
        gl_major_ver       = 4;
        gl_minor_ver       = 6;
        gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
    }
    else
    {
        gl_major_ver       = 3;
        gl_minor_ver       = 2;
        gl_context_profile = SDL_GL_CONTEXT_PROFILE_ES;
    }

    const char*      platform_from_sdl = SDL_GetPlatform();
    std::string_view platform(platform_from_sdl);

    using namespace std::string_view_literals;
    auto list_sys = { "Windows"sv, "Mac OS X"sv };
    auto it       = find(list_sys.begin(), list_sys.end(), platform);
    if (it != list_sys.end())
    {
        /* code */
        gl_major_ver       = 4;
        gl_minor_ver       = (platform == "Mac OS X") ? 1 : 3;
        gl_context_profile = SDL_GL_CONTEXT_PROFILE_CORE;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major_ver);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor_ver);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, gl_context_profile);

    gl_context = SDL_GL_CreateContext(window);

    {
        std::string profile;
        switch (gl_context_profile)
        {
            case SDL_GL_CONTEXT_PROFILE_CORE:
                profile = "CORE";
                break;
            case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
                profile = "COMPATIBILITY";
                break;
            case SDL_GL_CONTEXT_PROFILE_ES:
                profile = "ES";
                break;

            default:
                profile = "none";
                break;
        }

        std::clog << "OpenGl " << gl_major_ver << '.' << gl_minor_ver << " "
                  << profile << '\n';
    }

    if (gladLoadGLES2Loader(SDL_GL_GetProcAddress) == 0)
    {
        std::clog << "error: failed to initialize glad" << std::endl;
    }

#ifdef DEBUG_more
    if (platform != "Mac OS X") // not supported on Mac
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(callback_opengl_debug, nullptr);
        // glDebugMessageControl(
        //     GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    // RENDER_DOC///////////////////////////////////////////
    GLuint vertex_buffer = 0;
    glGenBuffers(1, &vertex_buffer);
    OM_GL_CHECK()
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    OM_GL_CHECK()
    GLuint vertex_array_object = 0;
    glGenVertexArrays(1, &vertex_array_object);
    OM_GL_CHECK()
    glBindVertexArray(vertex_array_object);
    OM_GL_CHECK()
    // RENDER_DOC///////////////////////////////////////////

    const std::string path("shader/");
    const std::string vert("test2.vert");
    const std::string frag("test2.frag");
    program_id_ = shader_create_program(path, vert, frag);

    /// turn on rendering with just created shader program
    glUseProgram(program_id_);
    OM_GL_CHECK()

    glEnable(GL_DEPTH_TEST);
    OM_GL_CHECK()

    return "";
}

bool engine_impl::read_input(my_engine::event& ev)
{
    // collect all events from SDL
    SDL_Event sdl_event;
    if (SDL_PollEvent(&sdl_event))
    {
        const bind* binding = nullptr;

        if (sdl_event.type == SDL_QUIT)
        {
            ev = event::turn_off;
            return true;
        }
        else if (sdl_event.type == SDL_KEYDOWN)
        {
            if (check_input(sdl_event, binding))
            {
                ev = binding->event_pressed;
                return true;
            }
        }
        else if (sdl_event.type == SDL_KEYUP)
        {
            if (check_input(sdl_event, binding))
            {
                ev = binding->event_released;
                return true;
            }
        }
    }
    return false;
}

void engine_impl::render_triangle(const triangle& t)
{
    // RENDER DOC addition ////////////////////
    glBufferData(GL_ARRAY_BUFFER, sizeof(t), &t, GL_DYNAMIC_DRAW);
    OM_GL_CHECK()
    glEnableVertexAttribArray(0);

    GLintptr position_attr_offset = 0;

    OM_GL_CHECK()
    glVertexAttribPointer(0,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(vertex),
                          reinterpret_cast<void*>(position_attr_offset));
    OM_GL_CHECK()
    glEnableVertexAttribArray(1);
    OM_GL_CHECK()

    GLintptr color_attr_offset = sizeof(float) * 3;

    glVertexAttribPointer(1,
                          3,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(vertex),
                          reinterpret_cast<void*>(color_attr_offset));
    OM_GL_CHECK()
    glValidateProgram(program_id_);
    OM_GL_CHECK()

    // Check the validate status
    GLint validate_status = 0;
    glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &validate_status);
    OM_GL_CHECK()
    if (validate_status == GL_FALSE)
    {
        GLint infoLen = 0;
        glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &infoLen);
        OM_GL_CHECK()
        std::vector<char> infoLog(static_cast<size_t>(infoLen));
        glGetProgramInfoLog(program_id_, infoLen, nullptr, infoLog.data());
        OM_GL_CHECK()
        std::cerr << "Error linking program:\n" << infoLog.data();
        throw std::runtime_error("error");
    }
    glDrawArrays(GL_TRIANGLES, 0, 3);
    OM_GL_CHECK()

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), &t.v[0]);
    // OM_GL_CHECK()
    // glEnableVertexAttribArray(0);
    // OM_GL_CHECK()
    // glValidateProgram(program_id_);
    // OM_GL_CHECK()
    // // Check the validate status
    // GLint validate_status = 0;
    // glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &validate_status);
    // OM_GL_CHECK()
    // if (validate_status == GL_FALSE)
    // {
    //     GLint infoLen = 0;
    //     glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &infoLen);
    //     OM_GL_CHECK()
    //     std::vector<char> infoLog(static_cast<size_t>(infoLen));
    //     glGetProgramInfoLog(program_id_, infoLen, nullptr, infoLog.data());
    //     OM_GL_CHECK()
    //     std::cerr << "Error linking program:\n" << infoLog.data();
    //     throw std::runtime_error("error");
    // }
    // glDrawArrays(GL_TRIANGLES, 0, 3);
    // OM_GL_CHECK()
}

void engine_impl::swap_buffers()
{
    SDL_GL_SwapWindow(window);

    glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
    OM_GL_CHECK()
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    OM_GL_CHECK()
}

void engine_impl::uninitialize()
{
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Create/destroy engine
static bool already_exist = false;

engine* create_engine()
{
    if (already_exist)
    {
        throw std::runtime_error("engine already exist");
    }
    engine* result = new engine_impl();
    already_exist  = true;
    return result;
}

void destroy_engine(engine* e)
{
    if (already_exist == false)
    {
        throw std::runtime_error("engine not created");
    }
    if (nullptr == e)
    {
        throw std::runtime_error("e is nullptr");
    }
    delete e;
}

engine::~engine() {}

// ERRORS
static const char* source_to_strv(GLenum source)
{
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            return "API";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "SHADER_COMPILER";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "WINDOW_SYSTEM";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "THIRD_PARTY";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "APPLICATION";
        case GL_DEBUG_SOURCE_OTHER:
            return "OTHER";
    }
    return "unknown";
}

static const char* type_to_strv(GLenum type)
{
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            return "ERROR";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "DEPRECATED_BEHAVIOR";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "UNDEFINED_BEHAVIOR";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "PERFORMANCE";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "PORTABILITY";
        case GL_DEBUG_TYPE_MARKER:
            return "MARKER";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "PUSH_GROUP";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "POP_GROUP";
        case GL_DEBUG_TYPE_OTHER:
            return "OTHER";
    }
    return "unknown";
}

static const char* severity_to_strv(GLenum severity)
{
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM:
            return "MEDIUM";
        case GL_DEBUG_SEVERITY_LOW:
            return "LOW";
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            return "NOTIFICATION";
    }
    return "unknown";
}

// 30Kb on my system, too much for stack
static std::array<char, GL_MAX_DEBUG_MESSAGE_LENGTH> local_log_buff;

static void APIENTRY
callback_opengl_debug(GLenum                       source,
                      GLenum                       type,
                      GLuint                       id,
                      GLenum                       severity,
                      GLsizei                      length,
                      const GLchar*                message,
                      [[maybe_unused]] const void* userParam)
{
    // The memory formessageis owned and managed by the GL, and should onlybe
    // considered valid for the duration of the function call.The behavior of
    // calling any GL or window system function from within thecallback function
    // is undefined and may lead to program termination.Care must also be taken
    // in securing debug callbacks for use with asynchronousdebug output by
    // multi-threaded GL implementations.  Section 18.8 describes thisin further
    // detail.

    auto& buff{ local_log_buff };
    int   num_chars = std::snprintf(buff.data(),
                                  buff.size(),
                                  "%s %s %d %s %.*s\n",
                                  source_to_strv(source),
                                  type_to_strv(type),
                                  id,
                                  severity_to_strv(severity),
                                  length,
                                  message);

    if (num_chars > 0)
    {
        // TODO use https://en.cppreference.com/w/cpp/io/basic_osyncstream
        // to fix possible data races
        // now we use GL_DEBUG_OUTPUT_SYNCHRONOUS to garantie call in main
        // thread
        std::cerr.write(buff.data(), num_chars);
    }
}

} // namespace my_engine