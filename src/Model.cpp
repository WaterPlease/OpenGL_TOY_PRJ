#include "Model.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <thread>
unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '\\' + filename;
    std::cout << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int TextureFromFile_NEAREST(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '\\' + filename;
    std::cout << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int TextureFromFile_ForHeight(const char* path, const string& directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '\\' + filename;
    std::cout << filename << std::endl;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void thread_img_load(std::vector<unsigned char*>& dataLst,int idx,char* filename, int *width, int *height, int *nrComponents) {
    unsigned char* data = stbi_load(filename, width, height, nrComponents, 0);
    if (!data) {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
    }
    dataLst[idx] = data;
}

unsigned int Texture3DFromFile(const std::vector<string> pathLst, const string& directory, bool gamma)
{
    std::vector<unsigned char*> dataLst;
    std::vector<std::thread> threadLst;
    GLenum format;
    int width, height, nrComponents;
    unsigned int textureID;
    unsigned int i = 0;

    dataLst.resize(pathLst.size());
    threadLst.resize(pathLst.size());
    /*for (i = 0; i < pathLst.size(); i++) {
        const char* path = pathLst[i].c_str();
        string filename = directory+std::string("\\") + string(path);
        dataLst[i] = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (!dataLst[i]) {
            std::cout << "Texture failed to load at path: " << filename << std::endl;
        }
        else {
            std::cout << filename << "\n";
        }
    }*/
    for (i = 0; i < pathLst.size(); i++) {
        threadLst[i] = std::thread(
            [&dataLst,pathLst,i,directory,&width,&height,&nrComponents]() {
                const char* path = pathLst[i].c_str();
                string filename = directory + std::string("\\") + string(path);
                dataLst[i] = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
                if (!dataLst[i]) {
                    std::cout << "Texture failed to load at path: " << filename << std::endl;
                }
            }
        );
    }
    for (i = 0; i < pathLst.size(); i++) {
        threadLst[i].join();
    }
    if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;
    std::cout << "FORMAT : " << nrComponents << "\n";
    std::cout << "WIDTH : " << width << "\n";
    std::cout << "HEIGHT : " << height << "\n";

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, pathLst.size(),0, format, GL_UNSIGNED_BYTE,NULL);

    i = 0;
    for (auto iter = dataLst.begin(); iter != dataLst.end(); iter++) {
        unsigned char* data = *iter;
        glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0,
            i, width, height, 1, format, GL_UNSIGNED_BYTE, data);
        i++;
        stbi_image_free(data);
    }

    return textureID;
}

unsigned int TextureCubeFromFile(const std::vector<string> pathLst, const string& directory, bool gamma) {
    // pathLst : +x -x +y -y +z -z
    unsigned int textureID;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    unsigned char* data;
    const std::string div = "\\";
    for (int i = 0; i < 6; i++) {
        data = stbi_load((directory+div+pathLst[i]).c_str(), &width, &height, &nrChannels,0);
        if (!data) {
            std::cout << "Cubemap tex failed to load at path: " << pathLst[i].c_str() << std::endl;
            stbi_image_free(data);
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}