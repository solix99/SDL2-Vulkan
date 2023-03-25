#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> 
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <KHR/khrplatform.h>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/SPIRV/spirv.hpp>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/resource_limits_c.h>

#include <vector>

class Shader
{
public:
    Shader(const char* vertexPath, const char* fragmentPath);


    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

    unsigned int ID;
};

#endif