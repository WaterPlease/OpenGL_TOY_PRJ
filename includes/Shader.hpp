#ifndef _SHADER_H_
#define _SHADER_H_

/////////////////////////////////////////////////////////////////////////////////
/*
*   This code is from learnopengl.com
*   you can find original code from :
*               https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/shader_s.h
* 
* 
*/


#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm_pre.h>
class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath, const char* tessControlPath=nullptr, const char* tessEvalPath=nullptr,const char* geometryPath=nullptr)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::string tessControlCode;
        std::string tessEvalCode;
        std::string geometryCode;

        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        std::ifstream tcShaderFile;
        std::ifstream teShaderFile;
        std::ifstream geometryFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        if (tessControlPath != nullptr && tessEvalPath != nullptr) {
            tcShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            teShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        }
        if (geometryPath != nullptr) {
            geometryFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        }
        try
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            if (tessControlPath != nullptr && tessEvalPath != nullptr) {
                tcShaderFile.open(tessControlPath);
                teShaderFile.open(tessEvalPath);
            }
            if (geometryPath != nullptr) {
                geometryFile.open(geometryPath);
            }
            std::stringstream vShaderStream, fShaderStream, tcShaderStream, teShaderStream, geometryStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            if (tessControlPath != nullptr && tessEvalPath != nullptr) {
                tcShaderStream << tcShaderFile.rdbuf();
                teShaderStream << teShaderFile.rdbuf();
            }
            if (geometryPath != nullptr) {
                geometryStream << geometryFile.rdbuf();
            }
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            if (tessControlPath != nullptr && tessEvalPath != nullptr) {
                tcShaderFile.close();
                teShaderFile.close();
            }
            if (geometryPath != nullptr) {
                geometryFile.close();
            }
            // convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
            if (tessControlPath != nullptr && tessEvalPath != nullptr) {
                tessControlCode = tcShaderStream.str();
                tessEvalCode = teShaderStream.str();
            }
            if (geometryPath != nullptr) {
                geometryCode = geometryStream.str();
            }
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment,tessControl, tessEval, geometry;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // tessellation Shader
        if (tessControlPath != nullptr && tessEvalPath != nullptr) {
            const char* tcShaderCode = tessControlCode.c_str();
            const char* teShaderCode = tessEvalCode.c_str();

            tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tessControl, 1, &tcShaderCode, NULL);
            glCompileShader(tessControl);
            checkCompileErrors(tessControl, "TESSCONTROL");

            tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tessEval, 1, &teShaderCode, NULL);
            glCompileShader(tessEval);
            checkCompileErrors(tessEval, "TESSEVAL");
        }
        if (geometryPath != nullptr) {
            const char* geoShaderCode = geometryCode.c_str();

            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &geoShaderCode, NULL);
            glCompileShader(geometry);
            checkCompileErrors(geometry, "GEOMETRY");
        }
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        if (tessControlPath != nullptr && tessEvalPath != nullptr) {
            glAttachShader(ID, tessControl);
            glAttachShader(ID, tessEval);
        }
        if (geometryPath != nullptr) {
            glAttachShader(ID, geometry);
        }
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (tessControlPath != nullptr && tessEvalPath != nullptr) {
            glDeleteShader(tessControl);
            glDeleteShader(tessEval);
        }
        if (geometryPath != nullptr) {
            glDeleteShader(geometry);
        }
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use()
    {
        glUseProgram(ID);
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
    void setVec3(const std::string& name, const glm::vec3& vec) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(vec));
    }

protected:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
    Shader() {}
};

class ComputeShader :public Shader {
    ComputeShader(const char* vertexPath, const char* fragmentPath, const char* tessControlPath = nullptr, const char* tessEvalPath = nullptr, const char* geometryPath = nullptr);
public:
    ComputeShader(const char* computePath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string computeCode;

        std::ifstream computeFile;
        // ensure ifstream objects can throw exceptions:
        computeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            // open files
            computeFile.open(computePath);
            std::stringstream computeStream;
            // read file's buffer contents into streams
            computeStream << computeFile.rdbuf();
            // close file handlers
            computeFile.close();
            // convert stream into string
            computeCode = computeStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* cShaderCode = computeCode.c_str();
        // 2. compile shaders
        unsigned int compute;
        // vertex shader
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "VERTEX");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(compute);
    }
    static void printWorkgrouInfo() {
        int work_grp_count[3];
        int work_grp_size[3];
        int work_grp_invc;

        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT,0, &work_grp_count[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_count[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_count[2]);
        
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE,0, &work_grp_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_invc);

        std::cout << "Maximum global work group count : (" << work_grp_count[0] << ", " << work_grp_count[1] << ", " << work_grp_count[2] << ")\n";
        std::cout << "Maximum local work group size : (" << work_grp_size[0] << ", " << work_grp_size[1] << ", " << work_grp_size[2] << ")\n";
        std::cout << "Maximum local work group invocation : " << work_grp_invc << "\n";
    }
};
#endif