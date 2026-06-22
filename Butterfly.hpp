#ifndef BUTTERFLY_HPP
#define BUTTERFLY_HPP

#include <GL/glut.h>
#include <string>
#include "ObjModel.hpp"

namespace ProjectButterfly
{

/**
 * Butterfly
 * Represents the butterfly healing particle/summon visual actor,
 * managing the OBJ models, textures, and wing-flapping animations.
 */
class Butterfly
{
private:
    // OBJ Model Parts
    ObjModel leftWingModel;
    ObjModel rightWingModel;
    ObjModel tursoModel;

    // Loading Flags
    bool leftWingLoaded;
    bool rightWingLoaded;
    bool tursoLoaded;

public:
    /**
     * Constructor that resets model loading flags and texture identifiers.
     */
    Butterfly();

    // Texture Bindings
    GLuint leftWingTextureID;
    GLuint rightWingTextureID;
    GLuint tursoTextureID;

    // OBJ File Loaders
    bool loadLeftWing(const std::string& filePath);
    bool loadRightWing(const std::string& filePath);
    bool loadTurso(const std::string& filePath);

    /**
     * Renders the left wing, applying the flapping rotation around the hinge axis.
     * flapAngle Current vertical wing flapping angle.
     */
    void drawLeftWing(float flapAngle = 0.0f) const;

    /**
     * Renders the right wing, applying the flapping rotation around the hinge axis.
     * flapAngle Current vertical wing flapping angle.
     */
    void drawRightWing(float flapAngle = 0.0f) const;

    /**
     * Renders the torso (Turso) of the butterfly.
     */
    void drawTurso() const;

    /**
     * Renders the complete butterfly hierarchically.
     * flapAngle Current vertical wing flapping angle.
     */
    void draw(float flapAngle = 0.0f) const;
};

} // namespace ProjectButterfly

#endif // BUTTERFLY_HPP
