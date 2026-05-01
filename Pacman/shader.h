#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>

class Shader {
public:
    GLuint Program;

    Shader(const char* vertexPath, const char* fragmentPath) {
        std::string vertCode, fragCode;
        std::ifstream vFile, fFile;
        vFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            vFile.open(vertexPath);
            fFile.open(fragmentPath);
            std::stringstream vss, fss;
            vss << vFile.rdbuf();
            fss << fFile.rdbuf();
            vFile.close(); fFile.close();
            vertCode = vss.str();
            fragCode = fss.str();
        } catch(std::ifstream::failure& e) {
            std::cerr << "ERROR::SHADER::FILE_NOT_READ: " << e.what() << std::endl;
        }

        const char* vCode = vertCode.c_str();
        const char* fCode = fragCode.c_str();

        GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");

        GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");

        Program = glCreateProgram();
        glAttachShader(Program, vertex);
        glAttachShader(Program, fragment);
        glLinkProgram(Program);
        checkCompileErrors(Program, "PROGRAM");

        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    void Use() { glUseProgram(Program); }

private:
    void checkCompileErrors(GLuint shader, std::string type) {
        GLint success;
        GLchar infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::SHADER::" << type << "::COMPILATION\n" << infoLog << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::PROGRAM::LINKING\n" << infoLog << std::endl;
            }
        }
    }
};