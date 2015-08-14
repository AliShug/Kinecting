#pragma once
#include "stdafx.h"
#include "Texture.h"

class ShaderManager {
public:
    class ParamRef {
    public:
        GLint ref = -1;

        ParamRef() = default;
        ParamRef(const GLint &r) : ref(r) {}

        // Validity
        inline bool valid() { return ref != -1; }

        // Binders
        inline void bindFloat(float in) {
            if (!valid()) throw std::exception("Attempted to bind invalid shader param");
            glUniform1f(ref, in);
        }
    };

    struct ParamMap {
        std::string name;
        ParamRef ref;

        ParamMap() = default;
        ParamMap(const std::string &n, GLint r)
            : name(n), ref(r) {}
    };

    ShaderManager() {}
    ~ShaderManager() {}

    void compileShaders();
    void use() { glUseProgram(programID); }

    void free() {
        if (programID != -1) glDeleteProgram(programID);
        programID = -1;
    }

    ParamRef namedParam(const std::string &name);
    void bindTexture(Texture &tex, const std::string &param);

    std::vector<ParamMap> parameters;

    GLuint programID = -1;
    char *vertexShader = "";
    char *fragmentShader = "";


protected:
    std::string _vShaderContents, _fShaderContents;
    GLint _vertexPos2DLoc, _depthImgLoc, _colImgLoc;
    GLint _xDimLoc, _yDimLoc;
    int textureInd = 0;

    void parseShaders();

    GLuint shaderFromFile(std::string file, GLenum type);
    std::string getShaderLog(GLuint shader);
    std::string getProgramLog(GLuint program);
    GLint fetchAttrLoc(const char* attr);
    GLint fetchUniformLoc(const char* uniform);
};

