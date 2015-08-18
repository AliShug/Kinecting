#include "stdafx.h"
#include "ShaderManager.h"
#include "Util.h"


void ShaderManager::compileShaders() {
    // Create the GL program
    programID = glCreateProgram();
	std::cout << "New program " << programID << std::endl;

    // Vertex shader
    GLuint vShade = shaderFromFile(vertexShader, GL_VERTEX_SHADER);
    if (vShade == 0) throw std::exception("Cannot load vertex shader");
    glAttachShader(programID, vShade);

    // Fragment shader
    GLuint fShade = shaderFromFile(fragmentShader, GL_FRAGMENT_SHADER);
    if (fShade == 0) throw std::exception("Cannot load fragment shader");
    glAttachShader(programID, fShade);

    // Link the GL program
    glLinkProgram(programID);
    GLint programLinked = GL_FALSE;
    glGetProgramiv(programID, GL_LINK_STATUS, &programLinked);
    if (programLinked != GL_TRUE) {
        std::cerr << "Failed to link GL program: " << getProgramLog(programID) << std::endl;
        throw std::exception("Shader link failure");
    }

    parseShaders();
}

ShaderManager::ParamRef ShaderManager::namedParam(const std::string &name) {
    for (int i = 0; i < parameters.size(); i++) {
        if (parameters[i].name == name) return parameters[i].ref;
    }

    return ParamRef();
    //throw std::exception(("Shader parameter " + name + " not found").c_str());
}

void ShaderManager::bindTexture(Texture & tex, const std::string &param) {
    ParamRef r = namedParam(param);

    if (r.valid()) {
        tex.bind(programID, namedParam(param).ref, textureInd++);
    }
    else {
        throw std::exception(("Texture binding error: Uniform " + param + "not found").c_str());
    }
}

void ShaderManager::parseShaders() {
    auto fullSrc = _vShaderContents + _fShaderContents;
    auto words = splitStr(fullSrc, "\n {};");

    for (auto it = words.begin(); it != words.end(); it++) {
        if (*it == "uniform") {
            auto it1 = it + 1;
            auto it2 = it + 2;

            GLint ref = fetchUniformLoc(it2->c_str());

            if (ref != -1) {
                parameters.push_back(ParamMap(*it2, ref));
            }
            else {
                std::cerr << "Warning: unable to locate " << *it2 << ", is it an unused parameter?" << std::endl;
                //throw std::exception(("Parsing: unable to locate " + *it2).c_str());
            }
        }
        else if (*it == "in") {
            auto it1 = it + 1;
            auto it2 = it + 2;

            GLint ref = fetchAttrLoc(it2->c_str());

            if (ref != -1) {
                parameters.push_back(ParamMap(*it2, ref));
            }
        }
    }
}

GLuint ShaderManager::shaderFromFile(std::string file, GLenum type) {
    // Load shader source file
    std::ifstream f("shaders/" + file);
    std::stringstream buffer;
    buffer << f.rdbuf();
    std::string shaderSrc = buffer.str();

    // Store
    switch (type) {
    case GL_VERTEX_SHADER:
        _vShaderContents = shaderSrc;
        break;
    case GL_FRAGMENT_SHADER:
        _fShaderContents = shaderSrc;
        break;
    }

    // Create the shader
    GLuint shader = glCreateShader(type);

    // Compile from source and check for errors
    const char* src = shaderSrc.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        std::cerr << "Unable to compile shader \"" << file << "\"\n" << getShaderLog(shader) << std::endl;
        return 0;
    }

    return shader;
}

std::string ShaderManager::getShaderLog(GLuint shader) {
    if (glIsShader(shader)) {
        // Log length
        int logLen = 0;
        int maxLen = logLen;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLen);

        // Alloc
        char* shaderLog = new char[maxLen];

        // Grab the info log
        glGetShaderInfoLog(shader, maxLen, &logLen, shaderLog);
        if (logLen > 0) {
            std::string outLog = shaderLog;
            delete shaderLog;
            return outLog;
        }
    }
    else {
        std::cerr << shader << " is not a shader." << std::endl;
    }

    return "";
}

std::string ShaderManager::getProgramLog(GLuint program) {
    if (glIsProgram(program)) {
        // Log length
        int logLen = 0;
        int maxLen = logLen;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLen);

        // Alloc
        char* programLog = new char[maxLen];

        // Grab the info log
        glGetProgramInfoLog(program, maxLen, &logLen, programLog);
        if (logLen > 0) {
            std::string outLog = programLog;
            delete programLog;
            return outLog;
        }
    }
    else {
        std::cerr << program << " is not a program." << std::endl;
    }

    return "";
}

GLint ShaderManager::fetchAttrLoc(const char* attr) {
    GLint loc = glGetAttribLocation(programID, attr);
    if (loc == -1) {
        std::cerr << "Unrecognized shader attribute: " << attr << std::endl;
        return -1;
    }

    return loc;
}

GLint ShaderManager::fetchUniformLoc(const char* uniform) {
    GLint loc = glGetUniformLocation(programID, uniform);
    if (loc == -1) {
        std::cerr << "Unrecognized shader uniform: " << uniform << std::endl;
        return -1;
    }

    return loc;
}