#pragma once

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h> 
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <SDL_vulkan.h>
#include <vulkan.h>
#include <vector>
#include <iostream>


using namespace std;

class Shader
{
public:
    Shader();

    static VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
    static std::vector<char> readFile(const std::string& filename);

private:

    //VkDevice DEVICE_M;

};

#endif