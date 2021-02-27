#include "../include/shader.hpp"

#include <fstream>
#include <iostream> // for DEBUG

void shader_loadFile(const std::string& path,
                     const std::string& file_name,
                     std::string*       result)
{
    std::string path_to_file = path + file_name;

    std::cout << path_to_file << "\tloading" << std::endl;

    std::ifstream file(path_to_file, std::ios_base::in | std::ios_base::ate);
    file.exceptions(std::ios_base::failbit);

    auto        size = file.tellg();
    std::string text(size, '\0');

    file.seekg(0);
    file.read(&text[0], size);
    *result = text;
}

GLuint shader_create_shader(const std::string path,
                            const std::string file_name,
                            GLuint            type)
{
    std::string shader_txt;
    shader_loadFile(path, file_name, &shader_txt);
    const char* txt    = shader_txt.data();
    GLuint      shader = glCreateShader(type);

    glShaderSource(shader, 1, &txt, nullptr);
    OM_GL_CHECK()

    glCompileShader(shader);
    OM_GL_CHECK()

    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    OM_GL_CHECK()
    if (!ok)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        GLchar log[infoLen];
        glGetShaderInfoLog(shader, infoLen, nullptr, log);
        std::cout << "\nERROR\n"
                  << "\ninfoLen: " << infoLen << "\n"
                  << log << std::endl;
    }

    return shader;
}

GLuint shader_create_program(const std::string path,
                             const std::string vertex_file_name,
                             const std::string fragment_file_name)
{
    GLuint vert_shader =
        shader_create_shader(path, vertex_file_name, GL_VERTEX_SHADER);
    OM_GL_CHECK()
    GLuint frag_shader =
        shader_create_shader(path, fragment_file_name, GL_FRAGMENT_SHADER);
    OM_GL_CHECK()

    GLuint prog = glCreateProgram();
    OM_GL_CHECK()

    glAttachShader(prog, vert_shader);
    OM_GL_CHECK()
    glAttachShader(prog, frag_shader);
    OM_GL_CHECK()

    // bind attribute location
    glBindAttribLocation(prog, 0, "a_position");
    OM_GL_CHECK()
    // link program after binding attribute locations
    glLinkProgram(prog);
    OM_GL_CHECK()

    // Check the link status
    GLint linked_status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked_status);
    OM_GL_CHECK()
    if (linked_status == 0)
    {
        GLint infoLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &infoLen);

        GLchar log[infoLen];
        glGetProgramInfoLog(prog, infoLen, nullptr, log);
        std::cout << "\nERROR\n" << log << std::endl;
    }
    
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return prog;
}