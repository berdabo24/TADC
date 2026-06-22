#ifndef TEXTURE_LOADER_HPP
#define TEXTURE_LOADER_HPP

#include <GL/glut.h>
#include <string>

class TextureLoader
{
public:
    static GLuint loadTexture(const std::string& path);
};

#endif
