#include <GL/glut.h>
#include "Butterfly.hpp"

using namespace ProjectButterfly;

/**
 * Constructor that resets model loading flags and texture identifiers.
 */
Butterfly::Butterfly()
{
    leftWingLoaded = false;
    rightWingLoaded = false;
    tursoLoaded = false;

    leftWingTextureID = 0;
    rightWingTextureID = 0;
    tursoTextureID = 0;
}

// ==========================================
// OBJ File Loaders
// ==========================================

bool Butterfly::loadLeftWing(const std::string& filePath)
{
    leftWingLoaded = leftWingModel.loadFromObjText(filePath);
    return leftWingLoaded;
}

bool Butterfly::loadRightWing(const std::string& filePath)
{
    rightWingLoaded = rightWingModel.loadFromObjText(filePath);
    return rightWingLoaded;
}

bool Butterfly::loadTurso(const std::string& filePath)
{
    tursoLoaded = tursoModel.loadFromObjText(filePath);
    return tursoLoaded;
}

// ==========================================
// Rendering Methods
// ==========================================

/**
 * Renders the left wing, applying the flapping rotation around the hinge axis.
 * flapAngle Current vertical wing flapping angle.
 */
void Butterfly::drawLeftWing(float flapAngle) const
{
    if (!leftWingLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftWingTextureID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    // Reorient model space
    glRotatef(90, 0, 1, 0);
    glScalef(3.0f, 3.0f, 3.0f);

    // Apply wing flap rotation around local longitudinal axis
    glRotatef(flapAngle, 1.0f, 0.0f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftWingModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

/**
 * Renders the right wing, applying the flapping rotation around the hinge axis.
 * flapAngle Current vertical wing flapping angle.
 */
void Butterfly::drawRightWing(float flapAngle) const
{
    if (!rightWingLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightWingTextureID);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();

    // Reorient model space
    glRotatef(90, 0, 1, 0);
    glScalef(3.0f, 3.0f, 3.0f);

    // Apply wing flap rotation around local longitudinal axis (inverted direction)
    glRotatef(-flapAngle, 1.0f, 0.0f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightWingModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

/**
 * Renders the torso (Turso) of the butterfly.
 */
void Butterfly::drawTurso() const
{
    if (!tursoLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tursoTextureID);

    glPushMatrix();

    // Reorient model space
    glRotatef(90, 0, 1, 0);
    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    tursoModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

/**
 * Renders the complete butterfly hierarchically.
 * flapAngle Current vertical wing flapping angle.
 */
void Butterfly::draw(float flapAngle) const
{
    drawLeftWing(flapAngle);
    drawRightWing(flapAngle);
    drawTurso();
}
