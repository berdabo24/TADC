#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <cmath>
#include "Environment.hpp"
#include "TextureLoader.hpp"
#include "CNAWorld.hpp"

using namespace ProjectEnvironment;

extern ProjectWorld::MyVirtualWorld myvirtualworld;
extern bool showHitboxes;
extern float boundaryScale;
extern float screenShakeIntensity;
extern float screenShakeTimer;
extern int difficultyLevel;
extern bool isTestArena;
extern int caineDeathSeqState;

static void getTransformedCenter(const ObjModel& model, float tx, float tz, float scale, float rotateAngleDegrees, float& outX, float& outZ);

// =======================Local environment object lighting====================== //
static void enableLocalEnvironmentLight(
    GLenum lightID,
    GLfloat x, GLfloat y, GLfloat z,
    GLfloat r, GLfloat g, GLfloat b,
    GLfloat linearAttenuation,
    GLfloat quadraticAttenuation
)
{
    glEnable(lightID);

    // 1.0f means point light, not directional light
    GLfloat lightPosition[] = { x, y, z, 1.0f };

    // Keep ambient very low because we only want local highlight
    GLfloat lightAmbient[]  = { 0.00f, 0.00f, 0.00f, 1.0f };
    GLfloat lightDiffuse[]  = { r, g, b, 1.0f };
    GLfloat lightSpecular[] = { r * 0.45f, g * 0.45f, b * 0.45f, 1.0f };

    glLightfv(lightID, GL_POSITION, lightPosition);
    glLightfv(lightID, GL_AMBIENT,  lightAmbient);
    glLightfv(lightID, GL_DIFFUSE,  lightDiffuse);
    glLightfv(lightID, GL_SPECULAR, lightSpecular);

    // Control how far the local light reaches
    glLightf(lightID, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(lightID, GL_LINEAR_ATTENUATION, linearAttenuation);
    glLightf(lightID, GL_QUADRATIC_ATTENUATION, quadraticAttenuation);
}

static void disableLocalEnvironmentLight(GLenum lightID)
{
    glDisable(lightID);
}
// ============================================================================ //

static void drawWireAABB(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    glBegin(GL_LINE_LOOP);
        glVertex3f(minX, minY, minZ);
        glVertex3f(maxX, minY, minZ);
        glVertex3f(maxX, minY, maxZ);
        glVertex3f(minX, minY, maxZ);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex3f(minX, maxY, minZ);
        glVertex3f(maxX, maxY, minZ);
        glVertex3f(maxX, maxY, maxZ);
        glVertex3f(minX, maxY, maxZ);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(minX, minY, minZ); glVertex3f(minX, maxY, minZ);
        glVertex3f(maxX, minY, minZ); glVertex3f(maxX, maxY, minZ);
        glVertex3f(maxX, minY, maxZ); glVertex3f(maxX, maxY, maxZ);
        glVertex3f(minX, minY, maxZ); glVertex3f(minX, maxY, maxZ);
    glEnd();
}

static void drawWireOBB(float tx, float ty, float tz, float scale, float rotateAngleDegrees, const Vec3& minB, const Vec3& maxB)
{
    glPushMatrix();
    glTranslatef(tx, ty, tz);
    if (std::abs(rotateAngleDegrees) > 0.001f)
    {
        glRotatef(rotateAngleDegrees, 0.0f, 1.0f, 0.0f);
    }
    drawWireAABB(minB.x * scale, maxB.x * scale, minB.y * scale, maxB.y * scale, minB.z * scale, maxB.z * scale);
    glPopMatrix();
}

static void drawWireCylinder(float cx, float cz, float radius, float minY, float maxY, int segments = 24)
{
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i)
    {
        float theta = 2.0f * 3.14159265f * float(i) / float(segments);
        glVertex3f(cx + radius * std::cos(theta), minY, cz + radius * std::sin(theta));
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i)
    {
        float theta = 2.0f * 3.14159265f * float(i) / float(segments);
        glVertex3f(cx + radius * std::cos(theta), maxY, cz + radius * std::sin(theta));
    }
    glEnd();

    glBegin(GL_LINES);
    for (int i = 0; i < segments; i += 4)
    {
        float theta = 2.0f * 3.14159265f * float(i) / float(segments);
        float x = cx + radius * std::cos(theta);
        float z = cz + radius * std::sin(theta);
        glVertex3f(x, minY, z);
        glVertex3f(x, maxY, z);
    }
    glEnd();
}

// Glitch Animation Helper Function
static void drawGlitchPanel(float width, float height, float uShift)
{
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f + uShift, 0.0f);
        glVertex3f(-width, -height, 0.0f);

        glTexCoord2f(1.0f + uShift, 0.0f);
        glVertex3f( width, -height, 0.0f);

        glTexCoord2f(1.0f + uShift, 1.0f);
        glVertex3f( width,  height, 0.0f);

        glTexCoord2f(0.0f + uShift, 1.0f);
        glVertex3f(-width,  height, 0.0f);
    glEnd();
}

Environment::Environment()
{
    // Objects
    skyBoxLoaded = false;
    groundLoaded = false;
    roofLoaded = false;
    castleWallLoaded = false;
    cubeLoaded = false;
    cubeGroupedLoaded = false;
    irregularCubeLoaded = false;
    pillarLoaded = false;
    sphereLoaded = false;

    // Render State Trackers
    isCubeDrawn = true;
    isPillarDrawn = true;
    isIrregularCubeDrawn = true;
    isCastleWallDrawn = true;

    // Animation
    animationTime = 0.0f;

    // Textures
    skyBoxTexture = 0;
    groundTexture = 0;
    roofTexture = 0;
    castleWallTexture = 0;
    cube1Texture = 0;
    cube2Texture = 0;
    cubeGroupedTexture = 0;
    circusObject1Texture = 0;
    circusObject2Texture = 0;
    pillarTexture = 0;
    skyTexture = 0;

    // Animation Textures
    glitchTexture = 0;

    // Initialize meteors (Sphere 1 to 5 original positions, scales, and bobs)
    meteors[0] = { 0.0f, 130.0f, 20.0f, 0.0f, 130.0f, 20.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 8.0f, 0.0f, 5.0f, 0, 0.0f, 0.0f, 0.0f };
    meteors[1] = { -60.0f, 110.0f, -40.0f, -60.0f, 110.0f, -40.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 8.0f, 1.2f, 4.0f, 0, 0.0f, 0.0f, 0.0f };
    meteors[2] = { 80.0f, 120.0f, -100.0f, 80.0f, 120.0f, -100.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 8.0f, 2.4f, 6.0f, 0, 0.0f, 0.0f, 0.0f };
    meteors[3] = { -90.0f, 140.0f, -160.0f, -90.0f, 140.0f, -160.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 8.0f, 3.6f, 5.0f, 0, 0.0f, 0.0f, 0.0f };
    meteors[4] = { 90.0f, 150.0f, 40.0f, 90.0f, 150.0f, 40.0f, 0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 8.0f, 4.8f, 4.0f, 0, 0.0f, 0.0f, 0.0f };

    isMeteorModeActive = false;
    meteorCooldownTimer = 0.0f;
    nextMeteorInterval = 10.0f;

    for (int i = 0; i < MAX_METEOR_PARTICLES; i++)
    {
        meteorParticles[i].active = false;
    }
}
bool Environment::loadTextures()
{
    skyBoxTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\CircusStrip.jpg"
    );

    groundTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\FloorStrip.png"
    );

    roofTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\Circus.jpg"
    );

    castleWallTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\CastleWall.jpg"
    );

    cube1Texture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\Cube1.png"
    );

    cube2Texture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\Cube2.png"
    );

    cubeGroupedTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\CubeGrouped.png"
    );

    circusObject1Texture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\CircusObject1.png"
    );

    circusObject2Texture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\CircusObject2.png"
    );

    pillarTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\Pillar.png"
    );

    skyTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\Sky.jpg"
    );

    // Animation Texture
    glitchTexture = TextureLoader::loadTexture(
        "Model\\Environment\\Textures\\GlitchEffect.png"
    );

    return skyBoxTexture != 0 &&
           groundTexture != 0 &&
           roofTexture != 0 &&
           castleWallTexture != 0 &&
           cube1Texture != 0 &&
           cube2Texture != 0 &&
           cubeGroupedTexture != 0 &&
           circusObject1Texture != 0 &&
           circusObject2Texture != 0 &&
           pillarTexture != 0 &&
           skyTexture != 0 &&
           glitchTexture != 0;
}

bool Environment::loadSkyBox(const std::string& filePath)
{
    skyBoxLoaded = skyBoxModel.loadFromObjText(filePath);
    return skyBoxLoaded;
}

bool Environment::loadGround(const std::string& filePath)
{
    groundLoaded = groundModel.loadFromObjText(filePath);
    return groundLoaded;
}

bool Environment::loadCastleWall(const std::string& filePath)
{
    castleWallLoaded = castleWallModel.loadFromObjText(filePath);
    return castleWallLoaded;
}

bool Environment::loadCube(const std::string& filePath)
{
    cubeLoaded = cubeModel.loadFromObjText(filePath);
    return cubeLoaded;
}

bool Environment::loadCubeGrouped(const std::string& filePath)
{
    cubeGroupedLoaded = cubeGroupedModel.loadFromObjText(filePath);
    return cubeGroupedLoaded;
}

bool Environment::loadIrregularCube(const std::string& filePath)
{
    irregularCubeLoaded = irregularCubeModel.loadFromObjText(filePath);
    return irregularCubeLoaded;
}

bool Environment::loadPillar(const std::string& filePath)
{
    pillarLoaded = pillarModel.loadFromObjText(filePath);
    return pillarLoaded;
}

bool Environment::loadRoof(const std::string& filePath)
{
    roofLoaded = roofModel.loadFromObjText(filePath);
    return roofLoaded;
}

bool Environment::loadSphere(const std::string& filePath)
{
    sphereLoaded = sphereModel.loadFromObjText(filePath);
    return sphereLoaded;
}

//////////////////////////////////Animation/////////////////////////////////
void Environment::tickTime() //Time-Based
{
    static int previousTime = -1;

    int currentTime = glutGet(GLUT_ELAPSED_TIME);

    if (previousTime < 0)
    {
        previousTime = currentTime;
        return;
    }

    float deltaTime = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    // Prevent a sudden big jump if the window freezes or pauses
    if (deltaTime > 0.1f)
    {
        deltaTime = 0.1f;
    }

    // Adjust this value if the animation is too slow or too fast
    const float animationSpeed = 1.0f;

    animationTime += deltaTime * animationSpeed;

    if (animationTime > 1000.0f)
    {
        animationTime = 0.0f;
    }

    // Reset flags at the end of update so that if draw() is not called next frame,
    // they remain false for the following tick.
    isCubeDrawn = false;
    isPillarDrawn = false;
    isIrregularCubeDrawn = false;
    isCastleWallDrawn = false;

    // Update meteors and their particles
    updateMeteors(deltaTime);
    updateMeteorParticles(deltaTime);
}

////////////////////////////////////Draw////////////////////////////////////
void Environment::drawSkyBox() const
{
    if (!skyBoxLoaded)
        return;

    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyBoxTexture);

    // Usually better to disable lighting for background
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Darken the whole background texture here
    glColor3ub(180, 180, 180);

    // Make sure texture color is multiplied by glColor
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // Size of SkyBox
    glScalef(15.0f, 15.0f, 15.0f);

    skyBoxModel.draw();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);

    glPopMatrix();
}

void Environment::drawGround() const
{
    if (!groundLoaded)
        return;

    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture color is not tinted
    glColor3ub(255, 255, 255);

    // Transformation
    glTranslatef(0.0f, -18.7f, 0.0f);
    glScalef(15.0f, 15.0f, 15.0f);

    groundModel.draw();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

void Environment::drawRoof() const
{
    if (!roofLoaded)
        return;

    glPushMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, roofTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // White color so the roof texture appears correctly
    glColor3ub(255, 255, 255);

    //////////////////////////////Animation////////////////////////////
    // Slow roof rotation
    float roofRotateAngle = animationTime * 8.0f;
    ////////////////////////////////////////////////////////////////////

    // Size and translate of the roof
    glTranslatef(0.0f, 18.7f, 0.0f);
    // Rotate around Y-axis
    glRotatef(roofRotateAngle, 0.0f, 1.0f, 0.0f);
    glScalef(15.0f, 15.0f, 15.0f);

    roofModel.draw();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

void Environment::drawCastleWall() const
{
    if (!castleWallLoaded)
        return;

    isCastleWallDrawn = true;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, castleWallTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture color is not tinted
    glColor3ub(255, 255, 255);

    // Back wall left
    glPushMatrix();
    glTranslatef(-66.5f, -18.7f, -66.5f);
    glScalef(10.0f, 15.0f, 10.0f);
    castleWallModel.draw();
    glPopMatrix();

    // Front wall left
    glPushMatrix();
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-66.5f, -18.7f, -66.5f);
    glScalef(10.0f, 15.0f, 10.0f);
    castleWallModel.draw();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void Environment::drawCube() const
{
    if (!cubeLoaded)
        return;

    isCubeDrawn = true;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture color is not tinted
    glColor3ub(255, 255, 255);

    //////////////////////////////Animation////////////////////////////
    // Left-right movement
    float moveX1 = sin(animationTime) * 20.0f;
    // Forward-backward movement
    float moveZ2 = cos(animationTime * 0.8f) * 25.0f;
    // Circular movement
    float circleX = sin(animationTime * 0.7f) * 25.0f;
    float circleZ = cos(animationTime * 0.7f) * 25.0f;
    // Slower left-right movement
    float moveX4 = sin(animationTime * 0.5f) * 30.0f;
    // Small rotation for all cubes
    float rotateAngle = animationTime * 25.0f;
    ////////////////////////////////////////////////////////////////////

    // =====================================================
    // Cube left 1 - Cube1.png
    // Moves left and right
    // =====================================================
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, cube1Texture);

    glTranslatef(-90.0f + moveX1, -18.7f, 100.0f);
    glRotatef(rotateAngle, 0.0f, 1.0f, 0.0f);
    glScalef(8.0f, 8.0f, 8.0f);

    cubeModel.draw();
    glPopMatrix();
    // =====================================================

    // =====================================================
    // Cube left 2 - Cube2.png
    // Moves forward and backward
    // =====================================================
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, cube2Texture);

    glTranslatef(-180.0f, -18.7f, 40.0f + moveZ2);
    glRotatef(-rotateAngle * 0.8f, 0.0f, 1.0f, 0.0f);
    glScalef(15.0f, 15.0f, 15.0f);

    cubeModel.draw();
    glPopMatrix();
    // =====================================================

    // =====================================================
    // Cube right 1 - Cube2.png
    // Circular movement
    // =====================================================
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, cube2Texture);

    glTranslatef(90.0f + circleX, -18.7f, -130.0f + circleZ);
    glRotatef(rotateAngle * 1.2f, 0.0f, 1.0f, 0.0f);
    glScalef(10.0f, 10.0f, 10.0f);

    cubeModel.draw();
    glPopMatrix();
    // =====================================================

    // =====================================================
    // Cube right 2 - Cube1.png again
    // Slower left and right movement
    // =====================================================
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, cube1Texture);

    glTranslatef(20.0f + moveX4, -18.7f, -210.0f);
    glRotatef(-rotateAngle * 0.6f, 0.0f, 1.0f, 0.0f);
    glScalef(17.0f, 17.0f, 17.0f);

    cubeModel.draw();
    glPopMatrix();
    // =====================================================

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void Environment::drawCubeGrouped() const
{
    if (!cubeGroupedLoaded)
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, cubeGroupedTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture color is not tinted
    glColor3ub(255, 255, 255);

    // Grouped block near center but slightly behind
    glPushMatrix();
    glTranslatef(220.0f, -18.7f, 150.0f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glScalef(15.0f, 15.0f, 15.0f);
    cubeGroupedModel.draw();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void Environment::drawIrregularCube() const
{
    if (!irregularCubeLoaded)
        return;

    isIrregularCubeDrawn = true;

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white color so the texture color appears correctly
    glColor3ub(255, 255, 255);

    // Abstract prop 1 - CircusObject1 texture
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, circusObject1Texture);
    glTranslatef(-42.0f, -18.7f, 0.0f);
    glScalef(18.0f, 18.0f, 18.0f);
    irregularCubeModel.draw();
    glPopMatrix();

    // Mirrored abstract prop 1 - CircusObject1 texture
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, circusObject1Texture);
    glTranslatef(42.0f, -18.7f, 0.0f);
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glScalef(18.0f, 18.0f, 18.0f);
    irregularCubeModel.draw();
    glPopMatrix();

    // Abstract prop 2 - CircusObject2 texture
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, circusObject2Texture);
    glTranslatef(-55.0f, -18.7f, 22.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(12.0f, 12.0f, 12.0f);
    irregularCubeModel.draw();
    glPopMatrix();

    // Mirrored abstract prop 2 - CircusObject2 texture
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, circusObject2Texture);
    glTranslatef(55.0f, -18.7f, -22.0f);
    glRotatef(270.0f, 0.0f, 1.0f, 0.0f);
    glScalef(12.0f, 12.0f, 12.0f);
    irregularCubeModel.draw();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void Environment::drawPillar() const
{
    if (!pillarLoaded)
        return;

    isPillarDrawn = true;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pillarTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture is not tinted
    glColor3ub(255, 255, 255);

    // Front-left pillar
    glPushMatrix();
    glTranslatef(-150.0f, -18.7f, 27.0f);
    glScalef(15.0f, 15.0f, 15.0f);
    pillarModel.draw();
    glPopMatrix();

    // Back-right pillar
    glPushMatrix();
    glTranslatef(150.0f, -18.7f, -270.0f);
    glScalef(15.0f, 15.0f, 15.0f);
    pillarModel.draw();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);
}

void Environment::drawSphere() const
{
    if (!sphereLoaded)
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, skyTexture);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    // Use white so the texture color appears correctly
    glColor3ub(255, 255, 255);

    float rotateAngle = animationTime * 30.0f;

    for (int i = 0; i < NUM_METEORS; i++)
    {
        const Meteor& m = meteors[i];
        if (m.state == 2) continue; // Impacted and hidden

        glPushMatrix();
        glTranslatef(m.currentX, m.currentY, m.currentZ);
        
        float scaleFactor = m.scale;
        glRotatef(rotateAngle * (0.6f + (i * 0.1f)), 0.0f, 1.0f, 0.0f);
        glScalef(scaleFactor, scaleFactor, scaleFactor);
        glTranslatef(0.0f, -3.755225f, 0.0f); // Center the sphere geometry around local origin
        
        sphereModel.draw();
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    // Draw active meteor trails and impact bursts
    drawMeteorParticles();
}

void Environment::drawGlitchPanelEffect(float x, float y, float z, float width, float height, float uShift, float alpha, float rotationAngle) const
{
    // Save current states
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, glitchTexture);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glDepthMask(GL_FALSE);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
    glPushMatrix();
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glTranslatef(x, y, z);
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f);
    
    drawGlitchPanel(width, height, uShift);
    
    glPopMatrix();
    
    glPopAttrib();
}

void Environment::drawThinGlitchLineEffect(float x, float y, float z, float length, float alpha, float dirX, float dirY, float dirZ) const
{
    // Save current states
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    glLineWidth(2.0f);
    glColor4f(0.0f, 0.9f, 1.0f, alpha);
    
    glBegin(GL_LINES);
        glVertex3f(x - dirX * length * 0.5f, y - dirY * length * 0.5f, z - dirZ * length * 0.5f);
        glVertex3f(x + dirX * length * 0.5f, y + dirY * length * 0.5f, z + dirZ * length * 0.5f);
    glEnd();
    
    glLineWidth(1.0f);
    
    glPopAttrib();
}

void Environment::drawDigitalEffect() const
{
    // =====================================================
    // Wider floating glitch panels across the environment
    // =====================================================
    for (int i = 0; i < 9; i++)
    {
        float x = -120.0f + (i % 3) * 120.0f;
        float z = -120.0f + (i / 3) * 100.0f;

        float y = 35.0f + (i % 3) * 12.0f
                + sin(animationTime * 1.5f + i) * 4.0f;

        float flicker = 0.12f + 0.35f * fabs(sin(animationTime * 7.0f + i));
        float uShift = sin(animationTime * 3.0f + i) * 0.20f;

        float width = 22.0f + (i % 2) * 12.0f;
        float height = 8.0f + (i % 3) * 4.0f;
        
        float rotationAngle = i * 35.0f + sin(animationTime + i) * 10.0f;

        drawGlitchPanelEffect(x, y, z, width, height, uShift, flicker, rotationAngle);
    }

    // =====================================================
    // More thin horizontal glitch scan lines
    // =====================================================
    for (int j = 0; j < 18; j++)
    {
        float x = -140.0f + (j % 6) * 55.0f;
        float z = -150.0f + (j / 6) * 100.0f;
        float y = 25.0f + (j % 4) * 14.0f;

        float moveX = sin(animationTime * 2.0f + j) * 8.0f;
        float blink = fabs(sin(animationTime * 9.0f + j));

        drawThinGlitchLineEffect(x + moveX, y, z, 35.0f, blink * 0.55f);
    }
}

/////////////////////////////////////Main draw function//////////////////////////
void Environment::draw() const
{
    // Reset rendering tracker flags at the start of rendering
    isCubeDrawn = false;
    isPillarDrawn = false;
    isIrregularCubeDrawn = false;
    isCastleWallDrawn = false;

    // Background first
    drawSkyBox();

    // =====================================================
    // Ground and roof: dim overhead stage light
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        0.0f, 80.0f, 0.0f,        // position
        0.35f, 0.35f, 0.45f,      // cool dim white-blue color
        0.002f,
        0.00001f
    );

    drawGround();
    drawRoof();

    disableLocalEnvironmentLight(GL_LIGHT1);


    // =====================================================
    // Castle walls: cold blue side light
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        -120.0f, 60.0f, -80.0f,   // position near left/back wall
        0.35f, 0.45f, 0.85f,      // blue lighting
        0.003f,
        0.00002f
    );

    drawCastleWall();

    disableLocalEnvironmentLight(GL_LIGHT1);


    // =====================================================
    // Cubes: red circus/battle light
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        70.0f, 60.0f, -80.0f,     // position near cube area
        0.90f, 0.20f, 0.20f,      // red light
        0.003f,
        0.00002f
    );

    drawCube();
    drawCubeGrouped();

    disableLocalEnvironmentLight(GL_LIGHT1);


    // =====================================================
    // Irregular circus objects: purple/magenta light
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        -50.0f, 45.0f, 40.0f,     // position near irregular cubes
        0.80f, 0.20f, 0.90f,      // purple light
        0.004f,
        0.00003f
    );

    drawIrregularCube();

    disableLocalEnvironmentLight(GL_LIGHT1);


    // =====================================================
    // Pillars: soft warm spotlight
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        100.0f, 90.0f, -170.0f,   // position near pillars
        0.85f, 0.75f, 0.55f,      // warm beige light
        0.003f,
        0.00002f
    );

    drawPillar();

    disableLocalEnvironmentLight(GL_LIGHT1);


    // =====================================================
    // Floating spheres: blue sky glow
    // =====================================================
    enableLocalEnvironmentLight(
        GL_LIGHT1,
        0.0f, 90.0f, 20.0f,       // position near middle/top
        0.25f, 0.55f, 1.00f,      // blue light
        0.003f,
        0.00002f
    );

    drawSphere();

    disableLocalEnvironmentLight(GL_LIGHT1);

    // Digital zap-zap circus effect (only drawn in debug environment)
    if (myvirtualworld.isDebugMode)
    {
        drawDigitalEffect();
    }

    // Draw collision wireframes if toggled visible
    if (showHitboxes)
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0f);

        // 1. Castle Walls (Red/Pink)
        if (castleWallLoaded && isCastleWallDrawn)
        {
            Vec3 minB, maxB;
            castleWallModel.getBounds(minB, maxB);

            float w1_minX = minB.x * 10.0f - 66.5f;
            float w1_maxX = maxB.x * 10.0f - 66.5f;
            float w1_minZ = minB.z * 10.0f - 66.5f;
            float w1_maxZ = maxB.z * 10.0f - 66.5f;
            if (w1_minX > w1_maxX) std::swap(w1_minX, w1_maxX);
            if (w1_minZ > w1_maxZ) std::swap(w1_minZ, w1_maxZ);

            float w2_minX = -(maxB.x * 10.0f - 66.5f);
            float w2_maxX = -(minB.x * 10.0f - 66.5f);
            float w2_minZ = -(maxB.z * 10.0f - 66.5f);
            float w2_maxZ = -(minB.z * 10.0f - 66.5f);
            if (w2_minX > w2_maxX) std::swap(w2_minX, w2_maxX);
            if (w2_minZ > w2_maxZ) std::swap(w2_minZ, w2_maxZ);

            glColor3f(1.0f, 0.3f, 0.3f);
            drawWireAABB(w1_minX, w1_maxX, -18.7f, 15.0f, w1_minZ, w1_maxZ);
            drawWireAABB(w2_minX, w2_maxX, -18.7f, 15.0f, w2_minZ, w2_maxZ);
        }

        // 2. Circus Map boundary limits (Cyan)
        if (skyBoxLoaded)
        {
            Vec3 skyMin, skyMax;
            skyBoxModel.getBounds(skyMin, skyMax);
            float bMinX = skyMin.x * boundaryScale;
            float bMaxX = skyMax.x * boundaryScale;
            float bMinZ = skyMin.z * boundaryScale;
            float bMaxZ = skyMax.z * boundaryScale;
            if (bMinX > bMaxX) std::swap(bMinX, bMaxX);
            if (bMinZ > bMaxZ) std::swap(bMinZ, bMaxZ);

            glColor3f(0.0f, 0.8f, 1.0f);
            drawWireAABB(bMinX, bMaxX, -18.7f, 40.0f, bMinZ, bMaxZ);
        }

        // 3. Interactive Obstacles (Green)
        glColor3f(0.0f, 1.0f, 0.0f);

        // 3a. Cubes (only if cubeLoaded and drawn)
        if (cubeLoaded && isCubeDrawn)
        {
            Vec3 minB, maxB;
            cubeModel.getBounds(minB, maxB);
            float hx = (maxB.x - minB.x) * 0.5f;
            float hz = (maxB.z - minB.z) * 0.5f;
            float localRadius = std::sqrt(hx * hx + hz * hz);
            float cubeRad = localRadius;

            float moveX1 = sin(animationTime) * 20.0f;
            float moveZ2 = cos(animationTime * 0.8f) * 25.0f;
            float circleX = sin(animationTime * 0.7f) * 25.0f;
            float circleZ = cos(animationTime * 0.7f) * 25.0f;
            float moveX4 = sin(animationTime * 0.5f) * 30.0f;
            float rotateAngle = animationTime * 25.0f;

            // Cube left 1
            float c1_x, c1_z;
            getTransformedCenter(cubeModel, -90.0f + moveX1, 100.0f, 8.0f, rotateAngle, c1_x, c1_z);
            drawWireCylinder(c1_x, c1_z, cubeRad * 8.0f, -18.7f, -18.7f + 16.0f);

            // Cube left 2
            float c2_x, c2_z;
            getTransformedCenter(cubeModel, -180.0f, 40.0f + moveZ2, 15.0f, -rotateAngle * 0.8f, c2_x, c2_z);
            drawWireCylinder(c2_x, c2_z, cubeRad * 15.0f, -18.7f, -18.7f + 30.0f);

            // Cube right 1
            float c3_x, c3_z;
            getTransformedCenter(cubeModel, 90.0f + circleX, -130.0f + circleZ, 10.0f, rotateAngle * 1.2f, c3_x, c3_z);
            drawWireCylinder(c3_x, c3_z, cubeRad * 10.0f, -18.7f, -18.7f + 20.0f);

            // Cube right 2
            float c4_x, c4_z;
            getTransformedCenter(cubeModel, 20.0f + moveX4, -210.0f, 17.0f, -rotateAngle * 0.6f, c4_x, c4_z);
            drawWireCylinder(c4_x, c4_z, cubeRad * 17.0f, -18.7f, -18.7f + 34.0f);
        }

        // 3b. Pillars (only if pillarLoaded and drawn)
        if (pillarLoaded && isPillarDrawn)
        {
            Vec3 minB, maxB;
            pillarModel.getBounds(minB, maxB);
            float hx = (maxB.x - minB.x) * 0.5f;
            float hz = (maxB.z - minB.z) * 0.5f;
            float localRadius = std::sqrt(hx * hx + hz * hz);
            float pillarRad = localRadius * 15.0f;

            float p1_x, p1_z;
            getTransformedCenter(pillarModel, -150.0f, 27.0f, 15.0f, 0.0f, p1_x, p1_z);
            drawWireCylinder(p1_x, p1_z, pillarRad, -18.7f, 15.0f);

            float p2_x, p2_z;
            getTransformedCenter(pillarModel, 150.0f, -270.0f, 15.0f, 0.0f, p2_x, p2_z);
            drawWireCylinder(p2_x, p2_z, pillarRad, -18.7f, 15.0f);
        }

        // 3c. Irregular Cubes (only if irregularCubeLoaded and drawn)
        if (irregularCubeLoaded && isIrregularCubeDrawn)
        {
            Vec3 minB, maxB;
            irregularCubeModel.getBounds(minB, maxB);

            drawWireOBB(-42.0f, -18.7f, 0.0f, 18.0f, 0.0f, minB, maxB);
            drawWireOBB(42.0f, -18.7f, 0.0f, 18.0f, 180.0f, minB, maxB);
            drawWireOBB(-55.0f, -18.7f, 22.0f, 12.0f, 90.0f, minB, maxB);
            drawWireOBB(55.0f, -18.7f, -22.0f, 12.0f, 270.0f, minB, maxB);
        }

        glPopAttrib();
    }
}

/**
 * Circle-AABB collision check with sliding response against the two castle wall sections.
 * Returns true if a collision was resolved, writing the new player position to outNewX and outNewZ.
 */
bool Environment::checkWallCollision(float playerX, float playerZ, float radius, float& outNewX, float& outNewZ) const
{
    if (!castleWallLoaded || !isCastleWallDrawn)
    {
        outNewX = playerX;
        outNewZ = playerZ;
        return false;
    }

    // Get local bounds of the castle wall
    Vec3 minB, maxB;
    castleWallModel.getBounds(minB, maxB);

    bool collided = false;
    outNewX = playerX;
    outNewZ = playerZ;

    // We have 2 walls:
    // Wall 1: Translation (-66.5, -18.7, -66.5), Scale (10, 15, 10)
    // Wall 2: Rotation 180, Translation (-66.5, -18.7, -66.5), Scale (10, 15, 10)
    
    // Bounding Box for Wall 1
    float w1_minX = minB.x * 10.0f - 66.5f;
    float w1_maxX = maxB.x * 10.0f - 66.5f;
    float w1_minZ = minB.z * 10.0f - 66.5f;
    float w1_maxZ = maxB.z * 10.0f - 66.5f;
    if (w1_minX > w1_maxX) std::swap(w1_minX, w1_maxX);
    if (w1_minZ > w1_maxZ) std::swap(w1_minZ, w1_maxZ);

    // Bounding Box for Wall 2
    // X_world = -X_temp, Z_world = -Z_temp
    // temp_minX = minB.x * 10 - 66.5, temp_maxX = maxB.x * 10 - 66.5
    // temp_minZ = minB.z * 10 - 66.5, temp_maxZ = maxB.z * 10 - 66.5
    float w2_minX = -(maxB.x * 10.0f - 66.5f);
    float w2_maxX = -(minB.x * 10.0f - 66.5f);
    float w2_minZ = -(maxB.z * 10.0f - 66.5f);
    float w2_maxZ = -(minB.z * 10.0f - 66.5f);
    if (w2_minX > w2_maxX) std::swap(w2_minX, w2_maxX);
    if (w2_minZ > w2_maxZ) std::swap(w2_minZ, w2_maxZ);

    // Resolve collision for both walls sequentially
    float boxes[2][4] = {
        {w1_minX, w1_maxX, w1_minZ, w1_maxZ},
        {w2_minX, w2_maxX, w2_minZ, w2_maxZ}
    };

    for (int i = 0; i < 2; ++i)
    {
        float bMinX = boxes[i][0];
        float bMaxX = boxes[i][1];
        float bMinZ = boxes[i][2];
        float bMaxZ = boxes[i][3];

        // Find closest point on AABB to player
        float cx = std::max(bMinX, std::min(outNewX, bMaxX));
        float cz = std::max(bMinZ, std::min(outNewZ, bMaxZ));

        float dx = outNewX - cx;
        float dz = outNewZ - cz;
        float dist = std::sqrt(dx * dx + dz * dz);

        if (dist < radius)
        {
            collided = true;
            if (dist > 0.00001f)
            {
                // Push player out of AABB
                outNewX = cx + (dx / dist) * radius;
                outNewZ = cz + (dz / dist) * radius;
            }
            else
            {
                // Player center is inside AABB, push to closest edge
                float distL = outNewX - bMinX;
                float distR = bMaxX - outNewX;
                float distB = outNewZ - bMinZ;
                float distT = bMaxZ - outNewZ;

                float minDist = std::min(std::min(distL, distR), std::min(distB, distT));
                if (minDist == distL) outNewX = bMinX - radius;
                else if (minDist == distR) outNewX = bMaxX + radius;
                else if (minDist == distB) outNewZ = bMinZ - radius;
                else outNewZ = bMaxZ + radius;
            }
        }
    }

    return collided;
}

static void getTransformedCenter(const ObjModel& model, float tx, float tz, float scale, float rotateAngleDegrees, float& outX, float& outZ)
{
    Vec3 localCenter = model.getCenter();
    if (std::abs(rotateAngleDegrees) < 0.001f)
    {
        outX = tx + localCenter.x * scale;
        outZ = tz + localCenter.z * scale;
    }
    else
    {
        float rad = rotateAngleDegrees * 3.14159265f / 180.0f;
        float rx = localCenter.x * std::cos(rad) + localCenter.z * std::sin(rad);
        float rz = -localCenter.x * std::sin(rad) + localCenter.z * std::cos(rad);
        outX = tx + rx * scale;
        outZ = tz + rz * scale;
    }
}

static bool checkOBBCollision(
    float playerX, float playerZ, float playerY, float radius,
    float tx, float tz, float scale, float rotateAngleDegrees,
    const Vec3& minB, const Vec3& maxB, float topY, float landingThreshold,
    bool isGloink, float& outNewX, float& outNewZ, float& outGroundY
) {
    // 1. Transform world player coordinates to local space relative to (tx, tz) rotated by rotateAngleDegrees
    float dx = playerX - tx;
    float dz = playerZ - tz;
    float rad = rotateAngleDegrees * 3.14159265f / 180.0f;
    float lx = dx * std::cos(rad) - dz * std::sin(rad);
    float lz = dx * std::sin(rad) + dz * std::cos(rad);

    // 2. Define local AABB limits
    float bMinX = minB.x * scale;
    float bMaxX = maxB.x * scale;
    float bMinZ = minB.z * scale;
    float bMaxZ = maxB.z * scale;
    if (bMinX > bMaxX) std::swap(bMinX, bMaxX);
    if (bMinZ > bMaxZ) std::swap(bMinZ, bMaxZ);

    // 3. Find closest point on local AABB
    float cx = std::max(bMinX, std::min(lx, bMaxX));
    float cz = std::max(bMinZ, std::min(lz, bMaxZ));

    float ldx = lx - cx;
    float ldz = lz - cz;
    float dist = std::sqrt(ldx * ldx + ldz * ldz);

    if (dist < radius)
    {
        if (isGloink || playerY >= topY - landingThreshold)
        {
            outGroundY = std::max(outGroundY, topY);
            return false; // climbed, no horizontal resolution
        }
        else
        {
            float newLx = lx;
            float newLz = lz;
            if (dist > 0.00001f)
            {
                newLx = cx + (ldx / dist) * radius;
                newLz = cz + (ldz / dist) * radius;
            }
            else
            {
                float distL = lx - bMinX;
                float distR = bMaxX - lx;
                float distB = lz - bMinZ;
                float distT = bMaxZ - lz;
                float minDist = std::min(std::min(distL, distR), std::min(distB, distT));
                if (minDist == distL) newLx = bMinX - radius;
                else if (minDist == distR) newLx = bMaxX + radius;
                else if (minDist == distB) newLz = bMinZ - radius;
                else newLz = bMaxZ + radius;
            }
            // Transform resolved local coordinates back to world space
            float rrad = rotateAngleDegrees * 3.14159265f / 180.0f;
            outNewX = tx + newLx * std::cos(rrad) + newLz * std::sin(rrad);
            outNewZ = tz - newLx * std::sin(rrad) + newLz * std::cos(rrad);
            return true; // collided and resolved
        }
    }
    return false;
}

static bool resolveCircleCollision(float playerX, float playerZ, float playerRadius,
                                   float obstacleX, float obstacleZ, float obstacleRadius,
                                   float& outNewX, float& outNewZ)
{
    float dx = playerX - obstacleX;
    float dz = playerZ - obstacleZ;
    float dist = std::sqrt(dx * dx + dz * dz);
    float minDist = playerRadius + obstacleRadius;

    if (dist < minDist)
    {
        if (dist > 0.00001f)
        {
            outNewX = obstacleX + (dx / dist) * minDist;
            outNewZ = obstacleZ + (dz / dist) * minDist;
        }
        else
        {
            outNewX = obstacleX + minDist;
            outNewZ = obstacleZ;
        }
        return true;
    }
    return false;
}

bool Environment::checkObstacleCollision(float playerX, float playerZ, float playerY, float radius, float& outNewX, float& outNewZ, float& outGroundY, bool isGloink) const
{
    bool collided = false;
    outNewX = playerX;
    outNewZ = playerZ;
    outGroundY = -18.7f;

    const float landingThreshold = 3.0f; // Allow landing if player's feet are within 3.0f units of the top surface

    // 1. Cubes (only if cubeLoaded and drawn)
    if (cubeLoaded && isCubeDrawn)
    {
        Vec3 minB, maxB;
        cubeModel.getBounds(minB, maxB);
        float hx = (maxB.x - minB.x) * 0.5f;
        float hz = (maxB.z - minB.z) * 0.5f;
        float localRadius = std::sqrt(hx * hx + hz * hz);
        float cubeRad = localRadius;

        float moveX1 = sin(animationTime) * 20.0f;
        float moveZ2 = cos(animationTime * 0.8f) * 25.0f;
        float circleX = sin(animationTime * 0.7f) * 25.0f;
        float circleZ = cos(animationTime * 0.7f) * 25.0f;
        float moveX4 = sin(animationTime * 0.5f) * 30.0f;
        float rotateAngle = animationTime * 25.0f;

        // Cube left 1 - Scale 8.0f, Translate (-90.0f + moveX1, -18.7f, 100.0f)
        float c1_x, c1_z;
        getTransformedCenter(cubeModel, -90.0f + moveX1, 100.0f, 8.0f, rotateAngle, c1_x, c1_z);
        float c1_rad = cubeRad * 8.0f;
        float c1_topY = -18.7f + maxB.y * 8.0f;
        float dx = outNewX - c1_x;
        float dz = outNewZ - c1_z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + c1_rad)
        {
            if (!isGloink && playerY >= c1_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, c1_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, c1_x, c1_z, c1_rad, outNewX, outNewZ))
                    collided = true;
            }
        }

        // Cube left 2 - Scale 15.0f, Translate (-180.0f, -18.7f, 40.0f + moveZ2)
        float c2_x, c2_z;
        getTransformedCenter(cubeModel, -180.0f, 40.0f + moveZ2, 15.0f, -rotateAngle * 0.8f, c2_x, c2_z);
        float c2_rad = cubeRad * 15.0f;
        float c2_topY = -18.7f + maxB.y * 15.0f;
        dx = outNewX - c2_x;
        dz = outNewZ - c2_z;
        dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + c2_rad)
        {
            if (!isGloink && playerY >= c2_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, c2_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, c2_x, c2_z, c2_rad, outNewX, outNewZ))
                    collided = true;
            }
        }

        // Cube right 1 - Scale 10.0f, Translate (90.0f + circleX, -18.7f, -130.0f + circleZ)
        float c3_x, c3_z;
        getTransformedCenter(cubeModel, 90.0f + circleX, -130.0f + circleZ, 10.0f, rotateAngle * 1.2f, c3_x, c3_z);
        float c3_rad = cubeRad * 10.0f;
        float c3_topY = -18.7f + maxB.y * 10.0f;
        dx = outNewX - c3_x;
        dz = outNewZ - c3_z;
        dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + c3_rad)
        {
            if (!isGloink && playerY >= c3_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, c3_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, c3_x, c3_z, c3_rad, outNewX, outNewZ))
                    collided = true;
            }
        }

        // Cube right 2 - Scale 17.0f, Translate (20.0f + moveX4, -18.7f, -210.0f)
        float c4_x, c4_z;
        getTransformedCenter(cubeModel, 20.0f + moveX4, -210.0f, 17.0f, -rotateAngle * 0.6f, c4_x, c4_z);
        float c4_rad = cubeRad * 17.0f;
        float c4_topY = -18.7f + maxB.y * 17.0f;
        dx = outNewX - c4_x;
        dz = outNewZ - c4_z;
        dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + c4_rad)
        {
            if (!isGloink && playerY >= c4_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, c4_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, c4_x, c4_z, c4_rad, outNewX, outNewZ))
                    collided = true;
            }
        }
    }

    // 2. Pillars (only if pillarLoaded and drawn)
    if (pillarLoaded && isPillarDrawn)
    {
        Vec3 minB, maxB;
        pillarModel.getBounds(minB, maxB);
        float hx = (maxB.x - minB.x) * 0.5f;
        float hz = (maxB.z - minB.z) * 0.5f;
        float localRadius = std::sqrt(hx * hx + hz * hz);
        float pillarRad = localRadius * 15.0f;
        float pillar_topY = -18.7f + maxB.y * 15.0f;

        // Front-left pillar - Scale 15.0f, Translate (-150.0f, -18.7f, 27.0f)
        float p1_x, p1_z;
        getTransformedCenter(pillarModel, -150.0f, 27.0f, 15.0f, 0.0f, p1_x, p1_z);
        float dx = outNewX - p1_x;
        float dz = outNewZ - p1_z;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + pillarRad)
        {
            if (!isGloink && playerY >= pillar_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, pillar_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, p1_x, p1_z, pillarRad, outNewX, outNewZ))
                    collided = true;
            }
        }

        // Back-right pillar - Scale 15.0f, Translate (150.0f, -18.7f, -270.0f)
        float p2_x, p2_z;
        getTransformedCenter(pillarModel, 150.0f, -270.0f, 15.0f, 0.0f, p2_x, p2_z);
        dx = outNewX - p2_x;
        dz = outNewZ - p2_z;
        dist = std::sqrt(dx * dx + dz * dz);
        if (dist < radius + pillarRad)
        {
            if (!isGloink && playerY >= pillar_topY - landingThreshold)
            {
                outGroundY = std::max(outGroundY, pillar_topY);
            }
            else
            {
                if (resolveCircleCollision(outNewX, outNewZ, radius, p2_x, p2_z, pillarRad, outNewX, outNewZ))
                    collided = true;
            }
        }
    }

    // 3. Irregular Cubes (only if irregularCubeLoaded and drawn)
    if (irregularCubeLoaded && isIrregularCubeDrawn)
    {
        Vec3 minB, maxB;
        irregularCubeModel.getBounds(minB, maxB);

        // Abstract prop 1 - Scale 18.0f, Translate (-42.0f, -18.7f, 0.0f)
        float ic1_topY = -18.7f + maxB.y * 18.0f;
        if (checkOBBCollision(outNewX, outNewZ, playerY, radius, -42.0f, 0.0f, 18.0f, 0.0f, minB, maxB, ic1_topY, landingThreshold, isGloink, outNewX, outNewZ, outGroundY))
            collided = true;

        // Mirrored Abstract prop 1 - Scale 18.0f, Translate (42.0f, -18.7f, 0.0f)
        float ic3_topY = -18.7f + maxB.y * 18.0f;
        if (checkOBBCollision(outNewX, outNewZ, playerY, radius, 42.0f, 0.0f, 18.0f, 180.0f, minB, maxB, ic3_topY, landingThreshold, isGloink, outNewX, outNewZ, outGroundY))
            collided = true;

        // Abstract prop 2 - Scale 12.0f, Translate (-55.0f, -18.7f, 22.0f), Rotated 90.0
        float ic2_topY = -18.7f + maxB.y * 12.0f;
        if (checkOBBCollision(outNewX, outNewZ, playerY, radius, -55.0f, 22.0f, 12.0f, 90.0f, minB, maxB, ic2_topY, landingThreshold, isGloink, outNewX, outNewZ, outGroundY))
            collided = true;

        // Mirrored Abstract prop 2 - Scale 12.0f, Translate (55.0f, -18.7f, -22.0f), Rotated 90.0
        float ic4_topY = -18.7f + maxB.y * 12.0f;
        if (checkOBBCollision(outNewX, outNewZ, playerY, radius, 55.0f, -22.0f, 12.0f, 270.0f, minB, maxB, ic4_topY, landingThreshold, isGloink, outNewX, outNewZ, outGroundY))
            collided = true;
    }

    return collided;
}

void Environment::getSkyBoxBounds(Vec3& minB, Vec3& maxB) const
{
    if (!skyBoxLoaded)
    {
        minB = {-20.0f, -20.0f, -20.0f};
        maxB = {20.0f, 20.0f, 20.0f};
        return;
    }
    skyBoxModel.getBounds(minB, maxB);
}

void Environment::resetMeteors()
{
    isMeteorModeActive = false;
    meteorCooldownTimer = 0.0f;
    nextMeteorInterval = 10.0f;
    
    for (int i = 0; i < NUM_METEORS; i++)
    {
        meteors[i].state = 0;
        meteors[i].currentX = meteors[i].originalX;
        meteors[i].currentY = meteors[i].originalY;
        meteors[i].currentZ = meteors[i].originalZ;
        meteors[i].fallProgress = 0.0f;
        meteors[i].respawnTimer = 0.0f;
    }

    for (int i = 0; i < MAX_METEOR_PARTICLES; i++)
    {
        meteorParticles[i].active = false;
    }
}

void Environment::updateMeteors(float deltaTime)
{
    // During Caine's death sequence, reset all active/falling meteors and bail out
    if (caineDeathSeqState > 0)
    {
        for (int i = 0; i < NUM_METEORS; i++)
        {
            meteors[i].state = 0;
            meteors[i].currentX = meteors[i].originalX;
            meteors[i].currentY = meteors[i].originalY;
            meteors[i].currentZ = meteors[i].originalZ;
            meteors[i].fallProgress = 0.0f;
        }
        for (int i = 0; i < MAX_METEOR_PARTICLES; i++)
        {
            meteorParticles[i].active = false;
        }
        return;
    }

    if (!isMeteorModeActive)
    {
        for (int i = 0; i < NUM_METEORS; i++)
        {
            meteors[i].state = 0;
            meteors[i].currentX = meteors[i].originalX;
            meteors[i].currentZ = meteors[i].originalZ;
            float floatY = std::sin(animationTime + meteors[i].floatOffset) * meteors[i].floatAmplitude;
            meteors[i].currentY = meteors[i].originalY + floatY;
        }
        return;
    }

    // Cooldown logic for launching meteors
    meteorCooldownTimer -= deltaTime;
    if (meteorCooldownTimer <= 0.0f)
    {
        meteorCooldownTimer = nextMeteorInterval;
        
        if (difficultyLevel == 2)
        {
            if (myvirtualworld.caine.currentPhase == 2)
            {
                nextMeteorInterval = 5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 5.0f));
            }
            else if (myvirtualworld.caine.currentPhase == 3)
            {
                nextMeteorInterval = 5.0f;
            }
            else
            {
                nextMeteorInterval = 5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 5.0f));
            }
        }
        else
        {
            nextMeteorInterval = 5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 20.0f));
        };
        
        int numToLaunch = (rand() % 2 == 0) ? 1 : 2;
        int launchedCount = 0;
        
        // Find meteors in state 0 (floating in sky)
        std::vector<int> candidates;
        for (int i = 0; i < NUM_METEORS; i++)
        {
            if (meteors[i].state == 0)
            {
                candidates.push_back(i);
            }
        }
        
        while (launchedCount < numToLaunch && !candidates.empty())
        {
            int randIdx = rand() % candidates.size();
            int mIdx = candidates[randIdx];
            candidates.erase(candidates.begin() + randIdx);
            
            meteors[mIdx].state = 1; // Falling
            meteors[mIdx].startX = meteors[mIdx].currentX;
            meteors[mIdx].startY = meteors[mIdx].currentY;
            meteors[mIdx].startZ = meteors[mIdx].currentZ;
            
            // Target the player's last known ground position
            meteors[mIdx].targetX = myvirtualworld.kinger.posX;
            meteors[mIdx].targetY = -18.7f;
            meteors[mIdx].targetZ = myvirtualworld.kinger.posZ;
            
            meteors[mIdx].fallProgress = 0.0f;
            meteors[mIdx].fallDuration = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 1.0f); // 1.5 to 2.5s
            
            launchedCount++;
        }
    }

    // Update meteors positions and states
    for (int i = 0; i < NUM_METEORS; i++)
    {
        Meteor& m = meteors[i];
        if (m.state == 0)
        {
            m.currentX = m.originalX;
            m.currentZ = m.originalZ;
            float floatY = std::sin(animationTime + m.floatOffset) * m.floatAmplitude;
            m.currentY = m.originalY + floatY;
        }
        else if (m.state == 1)
        {
            m.fallProgress += deltaTime / m.fallDuration;
            if (m.fallProgress >= 1.0f)
            {
                m.fallProgress = 1.0f;
                m.currentX = m.targetX;
                m.currentY = m.targetY;
                m.currentZ = m.targetZ;
                
                m.state = 2; // Impacted and hidden
                m.respawnTimer = 5.0f;
                
                spawnMeteorBurst(m.currentX, m.currentY, m.currentZ);
                
                // Trigger camera screen shake
                screenShakeTimer = 0.4f;
                screenShakeIntensity = 4.0f;
                
                // Damage and knockback collision checks against Kinger
                float dx = myvirtualworld.kinger.posX - m.currentX;
                float dz = myvirtualworld.kinger.posZ - m.currentZ;
                float dist = std::sqrt(dx * dx + dz * dz);
                float radius = 50.0f;
                
                if (dist < radius)
                {
                    float kDirX = 0.0f;
                    float kDirZ = 1.0f;
                    if (dist > 0.1f)
                    {
                        kDirX = dx / dist;
                        kDirZ = dz / dist;
                    }
                    
                    // Knockback Kinger
                    myvirtualworld.kinger.knockbackVelX = kDirX * 150.0f;
                    myvirtualworld.kinger.knockbackVelZ = kDirZ * 150.0f;
                    myvirtualworld.kinger.velocityY = 30.0f;
                    myvirtualworld.kinger.isGrounded = false;
                    
                    myvirtualworld.kinger.takeDamage(10);
                }

                // Damage and knockback all Gloinks within blast radius
                for (int gi = 0; gi < (int)myvirtualworld.gloinks.animation.activeGloinks.size(); gi++)
                {
                    auto& gloink = myvirtualworld.gloinks.animation.activeGloinks[gi];
                    if (gloink.isDead) continue;

                    float gdx = gloink.posX - m.currentX;
                    float gdz = gloink.posZ - m.currentZ;
                    float gdist = std::sqrt(gdx * gdx + gdz * gdz);

                    if (gdist < radius)
                    {
                        // Knockback direction away from impact point
                        float gDirX = 0.0f;
                        float gDirZ = 1.0f;
                        if (gdist > 0.1f)
                        {
                            gDirX = gdx / gdist;
                            gDirZ = gdz / gdist;
                        }

                        gloink.isKnockedBack = true;
                        gloink.knockbackTimer = 0.4f;
                        gloink.knockbackDirX = gDirX;
                        gloink.knockbackDirZ = gDirZ;

                        //myvirtualworld.gloinks.hurtGloink(gi);
                    }
                }
            }
            else
            {
                m.currentX = m.startX + (m.targetX - m.startX) * m.fallProgress;
                m.currentY = m.startY + (m.targetY - m.startY) * m.fallProgress;
                m.currentZ = m.startZ + (m.targetZ - m.startZ) * m.fallProgress;
                
                spawnMeteorTrailParticle(m.currentX, m.currentY, m.currentZ, m.scale);
            }
        }
        else if (m.state == 2)
        {
            m.respawnTimer -= deltaTime;
            if (m.respawnTimer <= 0.0f)
            {
                m.state = 0;
                m.currentX = m.originalX;
                m.currentY = m.originalY;
                m.currentZ = m.originalZ;
                m.fallProgress = 0.0f;
                
                spawnMeteorPoof(m.currentX, m.currentY, m.currentZ);
            }
        }
    }
}

void Environment::spawnMeteorTrailParticle(float x, float y, float z, float sizeFactor)
{
    int spawned = 0;
    for (int i = 0; i < MAX_METEOR_PARTICLES && spawned < 2; i++)
    {
        if (!meteorParticles[i].active)
        {
            meteorParticles[i].active = true;
            meteorParticles[i].posX = x + (-2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 4.0f));
            meteorParticles[i].posY = y + (-2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 4.0f));
            meteorParticles[i].posZ = z + (-2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 4.0f));
            
            meteorParticles[i].velX = -3.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 6.0f);
            meteorParticles[i].velY = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 5.0f);
            meteorParticles[i].velZ = -3.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 6.0f);
            
            meteorParticles[i].size = (sizeFactor * 0.1f) + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (sizeFactor * 0.15f));
            
            meteorParticles[i].r = 1.0f;
            meteorParticles[i].g = 0.3f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 0.5f);
            meteorParticles[i].b = 0.0f;
            
            meteorParticles[i].alpha = 1.0f;
            meteorParticles[i].lifeTime = 0.0f;
            meteorParticles[i].maxLife = 0.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 0.5f);
            
            spawned++;
        }
    }
}

void Environment::spawnMeteorBurst(float x, float y, float z)
{
    int spawned = 0;
    for (int i = 0; i < MAX_METEOR_PARTICLES && spawned < 50; i++)
    {
        if (!meteorParticles[i].active)
        {
            meteorParticles[i].active = true;
            meteorParticles[i].posX = x;
            meteorParticles[i].posY = y + 1.0f;
            meteorParticles[i].posZ = z;
            
            float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;
            float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.5707963f;
            float speed = 30.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 50.0f);
            
            meteorParticles[i].velX = speed * std::sin(phi) * std::cos(theta);
            meteorParticles[i].velY = speed * std::cos(phi);
            meteorParticles[i].velZ = speed * std::sin(phi) * std::sin(theta);
            
            meteorParticles[i].size = 1.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 3.5f);
            
            int type = rand() % 3;
            if (type == 0) {
                meteorParticles[i].r = 1.0f; meteorParticles[i].g = 0.5f; meteorParticles[i].b = 0.0f;
            } else if (type == 1) {
                meteorParticles[i].r = 1.0f; meteorParticles[i].g = 0.1f; meteorParticles[i].b = 0.1f;
            } else {
                meteorParticles[i].r = 1.0f; meteorParticles[i].g = 0.8f; meteorParticles[i].b = 0.0f;
            }
            
            meteorParticles[i].alpha = 1.0f;
            meteorParticles[i].lifeTime = 0.0f;
            meteorParticles[i].maxLife = 0.6f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 0.8f);
            
            spawned++;
        }
    }
}

void Environment::spawnMeteorPoof(float x, float y, float z)
{
    int spawned = 0;
    for (int i = 0; i < MAX_METEOR_PARTICLES && spawned < 15; i++)
    {
        if (!meteorParticles[i].active)
        {
            meteorParticles[i].active = true;
            meteorParticles[i].posX = x;
            meteorParticles[i].posY = y;
            meteorParticles[i].posZ = z;
            
            float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;
            float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.14159265f;
            float speed = 10.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 20.0f);
            
            meteorParticles[i].velX = speed * std::sin(phi) * std::cos(theta);
            meteorParticles[i].velY = speed * std::cos(phi);
            meteorParticles[i].velZ = speed * std::sin(phi) * std::sin(theta);
            
            meteorParticles[i].size = 1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 3.0f);
            
            int colType = rand() % 3;
            if (colType == 0) {
                meteorParticles[i].r = 0.0f; meteorParticles[i].g = 0.9f; meteorParticles[i].b = 1.0f;
            } else if (colType == 1) {
                meteorParticles[i].r = 1.0f; meteorParticles[i].g = 0.1f; meteorParticles[i].b = 0.6f;
            } else {
                meteorParticles[i].r = 1.0f; meteorParticles[i].g = 1.0f; meteorParticles[i].b = 1.0f;
            }
            
            meteorParticles[i].alpha = 1.0f;
            meteorParticles[i].lifeTime = 0.0f;
            meteorParticles[i].maxLife = 0.4f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 0.4f);
            
            spawned++;
        }
    }
}

void Environment::updateMeteorParticles(float deltaTime)
{
    for (int i = 0; i < MAX_METEOR_PARTICLES; i++)
    {
        if (meteorParticles[i].active)
        {
            meteorParticles[i].posX += meteorParticles[i].velX * deltaTime;
            meteorParticles[i].posY += meteorParticles[i].velY * deltaTime;
            meteorParticles[i].posZ += meteorParticles[i].velZ * deltaTime;
            
            meteorParticles[i].velX *= 0.95f;
            meteorParticles[i].velY *= 0.95f;
            meteorParticles[i].velZ *= 0.95f;
            
            meteorParticles[i].lifeTime += deltaTime;
            if (meteorParticles[i].lifeTime >= meteorParticles[i].maxLife)
            {
                meteorParticles[i].active = false;
            }
            else
            {
                meteorParticles[i].alpha = 1.0f - (meteorParticles[i].lifeTime / meteorParticles[i].maxLife);
            }
        }
    }
}

void Environment::drawMeteorParticles() const
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    for (int i = 0; i < MAX_METEOR_PARTICLES; i++)
    {
        const MeteorParticle& p = meteorParticles[i];
        if (p.active)
        {
            glPushMatrix();
            glTranslatef(p.posX, p.posY, p.posZ);
            
            float spin = p.lifeTime * 250.0f;
            glRotatef(spin, 1.0f, 1.0f, 0.0f);
            
            glColor4f(p.r, p.g, p.b, p.alpha);
            
            float s = p.size * 0.5f;
            glBegin(GL_QUADS);
                // Front
                glVertex3f(-s, -s,  s); glVertex3f( s, -s,  s); glVertex3f( s,  s,  s); glVertex3f(-s,  s,  s);
                // Back
                glVertex3f(-s, -s, -s); glVertex3f(-s,  s, -s); glVertex3f( s,  s, -s); glVertex3f( s, -s, -s);
                // Top
                glVertex3f(-s,  s, -s); glVertex3f(-s,  s,  s); glVertex3f( s,  s,  s); glVertex3f( s,  s, -s);
                // Bottom
                glVertex3f(-s, -s, -s); glVertex3f( s, -s, -s); glVertex3f( s, -s,  s); glVertex3f(-s, -s,  s);
                // Right
                glVertex3f( s, -s, -s); glVertex3f( s,  s, -s); glVertex3f( s,  s,  s); glVertex3f( s, -s,  s);
                // Left
                glVertex3f(-s, -s, -s); glVertex3f(-s, -s,  s); glVertex3f(-s,  s,  s); glVertex3f(-s,  s, -s);
            glEnd();
            
            glPopMatrix();
        }
    }
    glPopAttrib();
}
