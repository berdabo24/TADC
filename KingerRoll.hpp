#ifndef KINGER_ROLL_HPP
#define KINGER_ROLL_HPP

#include <GL/glut.h>
#include <string>
#include "ObjModel.hpp"

namespace ProjectKingerRoll
{

/**
 * KingerRoll
 * Represents the rolled-up checker ball model visual representation
 * used during Kinger's roll skill movement animation.
 */
class KingerRoll
{
private:
    ObjModel rollModel; // The OBJ loader instance for the roll ball model
    bool rollLoaded;    // Flag indicating if the roll model OBJ was loaded successfully

public:
    /**
     * Constructor that resets model loading flags and texture identifiers.
     */
    KingerRoll();

    GLuint rollTextureID; // Texture binding ID for the roll ball model

    /**
     * Loads the roll ball OBJ model from text content.
     * filePath File path or content identifier.
     */
    bool loadRoll(const std::string& filePath);

    /**
     * Draws the checker roll ball model, applying reorientation rotations and scaling.
     */
    void drawRoll() const;

    /**
     * Invokes the roll drawing method.
     */
    void draw() const;
};

} // namespace ProjectKingerRoll

#endif // KINGER_ROLL_HPP
