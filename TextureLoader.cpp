#include "TextureLoader.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint TextureLoader::loadTexture(const std::string& path)
{
    int width, height, channel;

    // Flip image vertically because OpenGL texture coordinate starts from bottom-left
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channel, 0);

    if (!data)
    {
        std::cout << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    extern void updateLoadingProgress(const std::string& action, const std::string& itemName);
    updateLoadingProgress("Loading Texture", path);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format;

    if (channel == 3)
    {
        format = GL_RGB;
    }
    else
    {
        format = GL_RGBA;
    }

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        width,
        height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        data
    );

    // Texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Texture wrapping
    // Use GL_CLAMP because some Code::Blocks / MinGW OpenGL headers
    // do not recognize GL_CLAMP_TO_EDGE.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(data);

    return textureID;
}
