#include <GL/glut.h>
#include "KingerRoll.hpp"

using namespace ProjectKingerRoll;

/**
 * Constructor that resets model loading flags and texture identifiers.
 */
KingerRoll::KingerRoll()
{
    rollLoaded = false;
    rollTextureID = 0;
}

/**
 * Loads the roll ball OBJ model from text content.
 * filePath File path or content identifier.
 */
bool KingerRoll::loadRoll(const std::string& filePath)
{
    rollLoaded = rollModel.loadFromObjText(filePath);
    return rollLoaded;
}

/**
 * Draws the checker roll ball model, applying reorientation rotations and scaling.
 */
void KingerRoll::drawRoll() const
{
    if (!rollLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rollTextureID);

    glPushMatrix();

    // Reorient model to align with parent rotation direction
    glRotatef(180, 0, 1, 0);
    glScalef(5.0f, 5.0f, 5.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rollModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

/**
 * Invokes the roll drawing method.
 */
void KingerRoll::draw() const
{
    drawRoll();
}
