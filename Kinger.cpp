#include <GL/glut.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include "Kinger.hpp"
#include "CNAWorld.hpp"

// Global scope reference to the main world instance
extern ProjectWorld::MyVirtualWorld myvirtualworld;
extern bool isTestArena;
extern int difficultyLevel;

using namespace ProjectKinger;

GLuint headTextureID;
static void drawSolidCubeFallback(float size)
{
    float h = size / 2.0f;

    glBegin(GL_QUADS);

    // Front
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-h, -h, h);
    glVertex3f( h, -h, h);
    glVertex3f( h,  h, h);
    glVertex3f(-h,  h, h);

    // Back
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f( h, -h, -h);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h,  h, -h);
    glVertex3f( h,  h, -h);

    // Left
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f(-h, -h,  h);
    glVertex3f(-h,  h,  h);
    glVertex3f(-h,  h, -h);

    // Right
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(h, -h,  h);
    glVertex3f(h, -h, -h);
    glVertex3f(h,  h, -h);
    glVertex3f(h,  h,  h);

    // Top
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-h, h,  h);
    glVertex3f( h, h,  h);
    glVertex3f( h, h, -h);
    glVertex3f(-h, h, -h);

    // Bottom
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-h, -h, -h);
    glVertex3f( h, -h, -h);
    glVertex3f( h, -h,  h);
    glVertex3f(-h, -h,  h);

    // Top
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-h, h,  h);
    glVertex3f( h, h,  h);
    glVertex3f( h, h, -h);
    glVertex3f(-h, h, -h);

    glEnd();
}

/**
 * Constructor that resets model loading flags, dimensions, properties, and health parameters.
 */
Kinger::Kinger()
{
    headLoaded = false;
    headPieceLoaded = false;
    leftEyeLoaded = false;
    rightEyeNLoaded = false;
    bodyLoaded = false;
    clothLoaded = false;
    leftHandLoaded = false;
    rightHandwGunLoaded = false;
    bucketLoaded = false;
    bucketHandleLoaded = false;

    currentHealth = 100;
    maxHealth = 100;

    posX      = 0.0f;
    posY      = -18.7f;
    currentGroundY = -18.7f;
    posZ      = 0.0f;
    facingYaw = 0.0f;

    targetLeanPitch  = 0.0f;
    currentLeanPitch = 0.0f;
    targetLeanRoll   = 0.0f;
    currentLeanRoll  = 0.0f;

    velocityY = 0.0f;
    knockbackVelX = 0.0f;
    knockbackVelZ = 0.0f;
    isGrounded = true;
    jumpScaleY = 1.0f;
    uniformScale = 1.0f;

    for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++)
    {
        muzzleParticles[i].active = false;
    }
}

/**
 * Sets the uniform scale of Kinger.
 * scale The scaling factor to apply uniformly.
 */
void Kinger::setScale(float scale)
{
    uniformScale = scale;
}

/**
 * Initiates the jump movement by applying vertical velocity.
 */
void Kinger::jump()
{
    if (animation.isDead) return;

    if (isGrounded)
    {
        velocityY = 50.0f;
        isGrounded = false;
        jumpScaleY = 1.2f;
    }
}

/**
 * Reduces health pool by a given amount and triggers hurt/death sequences.
 * amount Amount of damage to apply.
 */
void Kinger::takeDamage(int amount)
{
    if (isTestArena) return;

    if (animation.isHurt || animation.isRolling || animation.isDead) return;

    currentHealth -= amount;
    if (currentHealth <= 0)
    {
        currentHealth = 0;
        animation.triggerDeath();
    }
    else
    {
        animation.triggerHurt();
    }
}

/**
 * Resets health, velocities, coordinates, and transitions to normal idle alive state.
 */
void Kinger::rebirth()
{
    if (difficultyLevel == 0)
    {
        maxHealth = 50;
        animation.butterflyCharges = 5;
    }
    else if (difficultyLevel == 1)
    {
        maxHealth = 20;
        animation.butterflyCharges = 3;
    }
    else
    {
        maxHealth = 20;
        animation.butterflyCharges = 1;
    }
    currentHealth = maxHealth;
    posX = 0.0f;
    posY = -18.7f;
    currentGroundY = -18.7f;
    posZ = 0.0f;
    facingYaw = 0.0f;
    velocityY = 0.0f;
    knockbackVelX = 0.0f;
    knockbackVelZ = 0.0f;
    isGrounded = true;

    // Reset animation death state and charges
    animation.isDead = false;
    animation.deathTimer = 0.0f;
}

/**
 * Updates physics, camera aiming orientations, input movement, and animation states.
 * deltaTime The elapsed frame time in seconds.
 * cameraYaw Current camera orientation yaw angle.
 * cameraPitch Current camera orientation pitch angle.
 * keyStates Array tracking keyboard/button inputs.
 */
void Kinger::update(float deltaTime, float cameraYaw, float cameraPitch, const bool* keyStates)
{
    if (animation.isDead)
    {
        animation.updateDeathState(deltaTime);
        // Apply gravity during death falling
        velocityY += -50.0f * deltaTime;
        posY += velocityY * deltaTime;
        if (posY <= -18.7f)
        {
            posY = -18.7f;
            velocityY = 0.0f;
        }

        if (animation.deathTimer >= RESPAWN_DELAY)
        {
            if (::myvirtualworld.isDebugMode)
            {
                rebirth();
            }
        }
        return;
    }

    facingYaw = cameraYaw;
    aimPitch = -cameraPitch;

    if (keyStates[1])
    {
        if (!animation.isCastingSkill)
        {
            animation.castGunSkill();
        }
    }

    animation.updateIdleState(deltaTime);

    animation.updateSkillState(deltaTime, cameraYaw, cameraPitch, posX, posY, posZ, uniformScale);
    if (animation.shouldSpawnMuzzleFlash)
    {
        //Shotgun Sound Effect
        ::myvirtualworld.audioManager.playSoundEffect("Audio\\SFX\\Shotgun.wav");

        spawnMuzzleFlash(animation.bulletStartX, animation.bulletStartY, animation.bulletStartZ);
        animation.shouldSpawnMuzzleFlash = false;
    }
    updateMuzzleParticles(deltaTime);

    animation.updateRollState(deltaTime);
    animation.updateReloadState(deltaTime);
    animation.updateHealState(deltaTime);
    animation.updateHurtState(deltaTime);

    {
        // Apply knockback velocity
        posX += knockbackVelX * deltaTime;
        posZ += knockbackVelZ * deltaTime;
        knockbackVelX *= std::pow(0.1f, deltaTime);
        knockbackVelZ *= std::pow(0.1f, deltaTime);
        if (std::abs(knockbackVelX) < 0.01f) knockbackVelX = 0.0f;
        if (std::abs(knockbackVelZ) < 0.01f) knockbackVelZ = 0.0f;

        float fwd = 0.0f;
        float rgt = 0.0f;

        if (keyStates['w'] || keyStates['W']) fwd += 1.0f;
        if (keyStates['s'] || keyStates['S']) fwd -= 1.0f;
        if (keyStates['d'] || keyStates['D']) rgt += 1.0f;
        if (keyStates['a'] || keyStates['A']) rgt -= 1.0f;

        float magnitude = std::sqrt(fwd * fwd + rgt * rgt);
        if (magnitude > 0.0f)
        {
            float KINGER_INTERNAL_SPEED = 45.0f;

            if (animation.isRolling)
            {
                const float ROLL_SPEED_MULTIPLIER = 2.5f;
                KINGER_INTERNAL_SPEED *= ROLL_SPEED_MULTIPLIER;
            }

            if (animation.isHurt)
            {
                const float HURT_SPEED_MULTIPLIER = 0.5f;
                KINGER_INTERNAL_SPEED *= HURT_SPEED_MULTIPLIER;
            }

            float step = KINGER_INTERNAL_SPEED * deltaTime;

            float normFwd = fwd / magnitude;
            float normRgt = rgt / magnitude;

            posX += step * (normFwd * -std::sin(cameraYaw) + normRgt * std::cos(cameraYaw));
            posZ += step * (normFwd * -std::cos(cameraYaw) + normRgt * -std::sin(cameraYaw));
        }

        // Resolve wall collisions with sliding response
        float newX = posX;
        float newZ = posZ;
        const float collisionRadius = 3.0f * uniformScale;
        ::myvirtualworld.environment.checkWallCollision(posX, posZ, collisionRadius, newX, newZ);
        posX = newX;
        posZ = newZ;

        // Resolve dynamic and static obstacle collisions
        newX = posX;
        newZ = posZ;
        currentGroundY = -18.7f;
        ::myvirtualworld.environment.checkObstacleCollision(posX, posZ, posY, collisionRadius, newX, newZ, currentGroundY);
        posX = newX;
        posZ = newZ;

        // Clamp to map borders (circus walls) dynamically based on the skybox model's dimensions
        Vec3 skyMin, skyMax;
        ::myvirtualworld.environment.getSkyBoxBounds(skyMin, skyMax);

        float playerOffset = 3.0f * uniformScale;
        float limitMinX = skyMin.x * boundaryScale + playerOffset;
        float limitMaxX = skyMax.x * boundaryScale - playerOffset;
        float limitMinZ = skyMin.z * boundaryScale + playerOffset;
        float limitMaxZ = skyMax.z * boundaryScale - playerOffset;

        // Ensure bounds are valid in case values are inverted
        if (limitMinX > limitMaxX) std::swap(limitMinX, limitMaxX);
        if (limitMinZ > limitMaxZ) std::swap(limitMinZ, limitMaxZ);

        if (posX < limitMinX) posX = limitMinX;
        if (posX > limitMaxX) posX = limitMaxX;
        if (posZ < limitMinZ) posZ = limitMinZ;
        if (posZ > limitMaxZ) posZ = limitMaxZ;

        if (keyStates['w'] || keyStates['W']) {
            targetLeanPitch = MAX_LEAN_ANGLE;
        } else if (keyStates['s'] || keyStates['S']) {
            targetLeanPitch = -MAX_LEAN_ANGLE;
        } else {
            targetLeanPitch = 0.0f;
        }

        if (keyStates['a'] || keyStates['A']) {
            targetLeanRoll = -MAX_LEAN_ANGLE;
        } else if (keyStates['d'] || keyStates['D']) {
            targetLeanRoll = MAX_LEAN_ANGLE;
        } else {
            targetLeanRoll = 0.0f;
        }

        const float LEAN_SPEED = 3.0f;
        currentLeanPitch += (targetLeanPitch - currentLeanPitch) * LEAN_SPEED * deltaTime;
        currentLeanRoll  += (targetLeanRoll  - currentLeanRoll)  * LEAN_SPEED * deltaTime;
    }

    // Apply gravity constantly
    velocityY += -60.0f * deltaTime;
    posY += velocityY * deltaTime;

    if (posY <= currentGroundY)
    {
        posY = currentGroundY;
        velocityY = 0.0f;
        if (!isGrounded)
        {
            isGrounded = true;
            jumpScaleY = 0.8f;
        }
    }
    else
    {
        isGrounded = false;
    }

    jumpScaleY += (1.0f - jumpScaleY) * 10.0f * deltaTime;
}

/**
 * Shifts player position along the forward orientation vector.
 * speed Movement speed multiplier.
 * cameraYaw Reference direction angle.
 */
void Kinger::moveForward(float speed, float cameraYaw)
{
    posX -= speed * std::sin(cameraYaw);
    posZ -= speed * std::cos(cameraYaw);
}

/**
 * Shifts player position along the lateral side orientation vector.
 * speed Movement speed multiplier.
 * cameraYaw Reference direction angle.
 */
void Kinger::moveRight(float speed, float cameraYaw)
{
    posX += speed * std::cos(cameraYaw);
    posZ -= speed * std::sin(cameraYaw);
}

// ==========================================
// OBJ File Loaders
// ==========================================

bool Kinger::loadHead(const std::string& filePath)
{
    headLoaded = headModel.loadFromObjText(filePath);
    return headLoaded;
}

bool Kinger::loadHeadPiece(const std::string& filePath)
{
    headPieceLoaded = headPieceModel.loadFromObjText(filePath);
    return headPieceLoaded;
}

bool Kinger::loadLeftEye(const std::string& filePath)
{
    leftEyeLoaded = leftEyeModel.loadFromObjText(filePath);
    return leftEyeLoaded;
}

bool Kinger::loadRightEyeN(const std::string& filePath)
{
    rightEyeNLoaded = rightEyeNModel.loadFromObjText(filePath);
    return rightEyeNLoaded;
}

bool Kinger::loadBody(const std::string& filePath)
{
    bodyLoaded = bodyModel.loadFromObjText(filePath);
    return bodyLoaded;
}

bool Kinger::loadCloth(const std::string& filePath)
{
    clothLoaded = clothModel.loadFromObjText(filePath);
    return clothLoaded;
}

bool Kinger::loadLeftHand(const std::string& filePath)
{
    leftHandLoaded = leftHandModel.loadFromObjText(filePath);
    return leftHandLoaded;
}

bool Kinger::loadRightHandwGun(const std::string& filePath)
{
    rightHandwGunLoaded = rightHandwGunModel.loadFromObjText(filePath);
    return rightHandwGunLoaded;
}

bool Kinger::loadBucket(const std::string& filePath)
{
    bucketLoaded = bucketModel.loadFromObjText(filePath);
    return bucketLoaded;
}

bool Kinger::loadBucketHandle(const std::string& filePath)
{
    bucketHandleLoaded = bucketHandleModel.loadFromObjText(filePath);
    return bucketHandleLoaded;
}

// ==========================================
// Rendering Methods for Individual Parts
// ==========================================

/**
 * Render Kinger's main white chess piece head.
 */
void Kinger::drawHead() const
{
    if (!headLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, headTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glTranslatef(0.0f, 15.0f, 0.0f);
    glScalef(1.6f, 2.6f, 0.9f);

    glColor3f(1.0f, 1.0f, 1.0f);
    //glColor3ub(254, 226, 205);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    headModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Kinger's top headpiece crown.
 */
void Kinger::drawHeadPiece() const
{
    if (!headPieceLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, headPieceTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glTranslatef(0.0f, 14.5f, 0.0f);
    glScalef(1.2f, 2.0f, 0.7f);
    //glColor3ub(254, 226, 205);
    glColor3f(1.0f, 1.0f, 1.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    headPieceModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Kinger's left eye.
 */
void Kinger::drawLeftEye() const
{
    if (!leftEyeLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftEyeTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glTranslatef(6.0f, 28.0f, 0.0f);
    glScalef(8.0f, 8.0f, 8.0f);
    glRotatef(180, 1, 0, 0);
    glColor3ub(255, 255, 255);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftEyeModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Kinger's right eye.
 */
void Kinger::drawRightEyeN() const
{
    if (!rightEyeNLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightEyeNTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glRotatef(180, 1, 0, 0);
    glTranslatef(-7.0f, -32.0f, 2.0f);
    glScalef(7.5f, 7.5f, 7.5f);

    glColor3ub(255, 255, 255);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightEyeNModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Kinger's torso body connector.
 */
void Kinger::drawBody() const
{
    if (!bodyLoaded) return;

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    //glTranslatef(0.0f, 7.90f, 0.0f);
    glScalef(3.0f, 3.0f, 3.0f);
    glColor3ub(254, 226, 205);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    bodyModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

/**
 * Render Kinger's dynamic flowing cape cloth.
 */
void Kinger::drawCloth() const
{
    if (!clothLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, clothTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glRotatef(animation.clothRotation, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.0f, -18.0f, 0.0f);
    glScalef(7.5f, 8.0f, 7.0f);
    //glColor3ub(128, 0, 128);
    glColor3f(1.0f, 1.0f, 1.0f);
    glRotatef(180, 0, 1, 0);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    clothModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Kinger's left hand, managing reload, shooting, and heal rotations.
 */
void Kinger::drawLeftHand() const
{
    if (!leftHandLoaded) return;

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset + animation.leftArmReloadYOffset, animation.skillBodyZOffset);

    glRotatef(animation.leftArmReloadYaw, 0.0f, 1.0f, 0.0f);
    glRotatef(animation.leftArmReloadPitch, 1.0f, 0.0f, 0.0f);

    // Apply active healing gesture rotations
    glRotatef(animation.leftArmHealPitch, 1.0f, 0.0f, 0.0f);

    // Blend arm aiming vertical angles based on aim direction
    float visualPitch = aimPitch;
    if (visualPitch < -0.7f) visualPitch = -0.7f;
    if (visualPitch >  1.2f) visualPitch =  1.2f;
    float gunAimPitch = (-visualPitch * 57.2957795f);

    float aimBlend = (animation.leftArmReloadYaw / -135.0f);
    if (aimBlend < 0.0f) aimBlend = 0.0f;
    if (aimBlend > 1.0f) aimBlend = 1.0f;

    glRotatef(gunAimPitch * aimBlend, 1.0f, 0.0f, 0.0f);

    // Apply basic idle sway
    glRotatef(-animation.armRotation, 1.0f, 0.0f, 0.0f);
    //glRotatef(180,0,0,1); //initial position rotation
    glRotatef(150,0,0,1); //adjust position rotation
    glRotatef(180,1,0,0);
    glTranslatef(0.0f, -18.0f, 0.0f);
    glScalef(7.0f, 7.5f, 7.0f);
    glColor3ub(255, 255, 255);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftHandModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

/**
 * Render Kinger's right hand with weapon, applying recoil, reload, and idle sway.
 */
void Kinger::drawRightHandwGun() const
{
    if (!rightHandwGunLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightHandwGunTextureID);

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);

    // Apply reload rotations
    glRotatef(animation.rightArmReloadYaw, 0.0f, 1.0f, 0.0f);
    glRotatef(animation.rightArmReloadPitch, 1.0f, 0.0f, 0.0f);

    // Apply shooting recoil translation
    glTranslatef(0.0f, 0.0f, animation.armRecoilOffset);

    // Apply camera vertical aiming pitch + idle sway rotation
    float visualPitch = aimPitch;
    if (visualPitch < -0.7f) visualPitch = -0.7f;
    if (visualPitch >  1.2f) visualPitch =  1.2f;

    float finalArmPitch = (-visualPitch * 57.2957795f) + animation.armRotation;
    glRotatef(finalArmPitch, 1.0f, 0.0f, 0.0f);

    glRotatef(180, 1, 0, 0);
    glRotatef(180, 0, 0, 1);
    glTranslatef(0.0f, -18.0f, -5.0f);
    glScalef(7.0f, 7.5f, 7.0f);
    glColor3ub(255, 255, 255);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightHandwGunModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render the bucket item.
 */
void Kinger::drawBucket() const
{
    if (!bucketLoaded) return;

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glRotatef(-25, 1, 0, 0);
    glTranslatef(8.3f, 4.0f, 4.0f);
    glRotatef(180, 0, 0, 1);
    glScalef(0.6f, 0.4f, 0.6f);
    glColor3ub(244, 244, 244);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    bucketModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

/**
 * Render the bucket handle connector.
 */
void Kinger::drawBucketHandle() const
{
    if (!bucketHandleLoaded) return;

    glPushMatrix();
    glTranslatef(0.0f, animation.hoverOffset + animation.skillBodyYOffset, animation.skillBodyZOffset);
    glTranslatef(0.0f, 5.0f, 2.0f);
    glRotatef(90, 0, 1, 0);
    glRotatef(90, 0, 0, 1);
    glRotatef(-45, 0, 0, 1);
    glScalef(8.5f, 8.5f, 8.5f);
    //glColor3ub(254, 226, 205);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    bucketHandleModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

/**
 * Render the active bullet projectile particle and tracer path.
 */
void Kinger::drawBullet() const
{
    if (!animation.isBulletActive) return;

    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(3.0f);
    glDisable(GL_TEXTURE_2D);

    glColor4f(1.0f, 0.8f, 0.0f, 0.6f);

    // Draw tracer line
    glBegin(GL_LINES);
        glVertex3f(animation.bulletStartX, animation.bulletStartY, animation.bulletStartZ);
        glVertex3f(animation.bulletPosX, animation.bulletPosY, animation.bulletPosZ);
    glEnd();

    glPopAttrib();

    glPushMatrix();
    glTranslatef(animation.bulletPosX, animation.bulletPosY, animation.bulletPosZ);
    glDisable(GL_TEXTURE_2D);
    glColor3ub(255, 200, 0);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);

    static GLUquadricObj* bulletQuadric = NULL;
    if (bulletQuadric == NULL)
    {
        bulletQuadric = gluNewQuadric();
        gluQuadricNormals(bulletQuadric, GLU_SMOOTH);
    }

    gluSphere(bulletQuadric, 0.3, 10, 10);

    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
}

void Kinger::spawnMuzzleFlash(float x, float y, float z)
{
    int spawned = 0;
    for (int i = 0; i < MAX_MUZZLE_PARTICLES && spawned < 15; i++)
    {
        if (!muzzleParticles[i].active)
        {
            muzzleParticles[i].active = true;
            muzzleParticles[i].posX = x;
            muzzleParticles[i].posY = y;
            muzzleParticles[i].posZ = z;

            // Random direction in a sphere
            float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;
            float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.14159265f;
            float speed = 10.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 20.0f; // 10 to 30 units/sec

            muzzleParticles[i].velX = speed * std::sin(phi) * std::cos(theta);
            muzzleParticles[i].velY = speed * std::cos(phi);
            muzzleParticles[i].velZ = speed * std::sin(phi) * std::sin(theta);

            // Muzzle flash color (Yellow / Orange spectrum)
            // Red is always 1.0f. Green varies to create orange/yellow. Blue is 0.0f.
            float gVal = 0.3f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.6f; // 0.3 to 0.9
            muzzleParticles[i].r = 1.0f;
            muzzleParticles[i].g = gVal;
            muzzleParticles[i].b = 0.0f;

            muzzleParticles[i].size = 1.5f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f; // 1.5 to 3.5
            muzzleParticles[i].alpha = 1.0f;
            muzzleParticles[i].lifeTime = 0.0f;
            muzzleParticles[i].maxLife = 0.15f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.15f; // 0.15 to 0.30 seconds

            spawned++;
        }
    }
}

void Kinger::updateMuzzleParticles(float deltaTime)
{
    for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++)
    {
        if (muzzleParticles[i].active)
        {
            muzzleParticles[i].posX += muzzleParticles[i].velX * deltaTime;
            muzzleParticles[i].posY += muzzleParticles[i].velY * deltaTime;
            muzzleParticles[i].posZ += muzzleParticles[i].velZ * deltaTime;

            // Air resistance
            muzzleParticles[i].velX *= 0.92f;
            muzzleParticles[i].velY *= 0.92f;
            muzzleParticles[i].velZ *= 0.92f;

            muzzleParticles[i].lifeTime += deltaTime;
            if (muzzleParticles[i].lifeTime >= muzzleParticles[i].maxLife)
            {
                muzzleParticles[i].active = false;
            }
            else
            {
                // Fade out alpha
                float progress = muzzleParticles[i].lifeTime / muzzleParticles[i].maxLife;
                muzzleParticles[i].alpha = 1.0f - progress;

                // Grow size slightly as they expand, then shrink
                if (progress < 0.2f)
                {
                    muzzleParticles[i].size += deltaTime * 8.0f;
                }
                else
                {
                    muzzleParticles[i].size -= deltaTime * 4.0f;
                    if (muzzleParticles[i].size < 0.1f) muzzleParticles[i].size = 0.1f;
                }
            }
        }
    }
}

void Kinger::drawMuzzleParticles() const
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    for (int i = 0; i < MAX_MUZZLE_PARTICLES; i++)
    {
        if (muzzleParticles[i].active)
        {
            glPushMatrix();
            glTranslatef(muzzleParticles[i].posX, muzzleParticles[i].posY, muzzleParticles[i].posZ);

            // Spin particles for a dynamic look
            float spin = muzzleParticles[i].lifeTime * 250.0f;
            glRotatef(spin, 1.0f, 1.0f, 0.0f);

            glColor4f(muzzleParticles[i].r, muzzleParticles[i].g, muzzleParticles[i].b, muzzleParticles[i].alpha);

            // Draw a solid cube
            float s = muzzleParticles[i].size * 0.5f;
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

/**
 * Renders the entire hierarchical character structure, combining limbs, bodies, and items.
 */
void Kinger::draw() const
{
    glPushMatrix();

    // Position player in the world
    glTranslatef(posX, posY + 18.7f, posZ);

    const float RAD_TO_DEG = 57.2957795f;
    const float MODEL_FACING_OFFSET = 3.14159265f;
    glRotatef((facingYaw + MODEL_FACING_OFFSET) * RAD_TO_DEG, 0.0f, 1.0f, 0.0f);

    // Apply uniform scale
    glScalef(uniformScale, uniformScale, uniformScale);

    // This corrective translation adjusts Kinger's vertical position after scaling.
    // It ensures the model's base stays on the ground plane (posY) instead of floating.
    glTranslatef(0.0f, 18.7f * (1.0f - (1.0f / uniformScale)), 0.0f);

    // Apply Minecraft-style death animation: fall stiffly to the right side
    if (animation.isDead)
    {
        float progress = animation.deathTimer / DEATH_DURATION;
        if (progress > 1.0f) progress = 1.0f;
        float fallAngle = progress * 90.0f; // 0 to 90 degrees

        glTranslatef(0.0f, -18.7f, 0.0f);
        glRotatef(fallAngle, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f, 18.7f, 0.0f);
    }

    bool shouldRender = true;
    if (animation.isHurt)
    {
        // Toggle visibility every 0.05 seconds (10 Hz blink rate)
        if (std::fmod(animation.hurtTimer, 0.1f) < 0.05f)
        {
            shouldRender = false;
        }
    }

    if (shouldRender)
    {
        if (animation.isHurt || animation.isDead)
        {
            glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT);
            glDisable(GL_TEXTURE_2D);

            GLfloat redColor[]   = { 1.0f, 0.0f, 0.0f, 1.0f };
            GLfloat blackColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redColor);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redColor);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redColor);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blackColor);

            glColor3f(1.0f, 0.0f, 0.0f);
        }

        // Render as checker roll ball if rolling ability is active
        if (animation.showBallModel)
        {
            glPushMatrix();
            float rollSpin = animation.rollTimer * 1500.0f;
            glRotatef(rollSpin, 1.0f, 0.0f, 0.0f);

            ::myvirtualworld.kingerRoll.draw();
            glPopMatrix();
        }
        else
        {
            glPushMatrix();

            // Apply squash and stretch scaling
            glScalef(1.0f, jumpScaleY * animation.rollSquashY, 1.0f);

            // Apply tilt lean based on movement direction
            glRotatef(currentLeanPitch, 1.0f, 0.0f, 0.0f);

            drawHead();
            drawHeadPiece();
            drawLeftEye();
            drawRightEyeN();
            drawBody();
            drawCloth();
            drawBucket();
            drawBucketHandle();

            glPopMatrix();

            glPushMatrix();

            drawLeftHand();

            glRotatef(25, 1.0f, 0.0f, 0.0f);
            drawRightHandwGun();
            glRotatef(-25, 1.0f, 0.0f, 0.0f);

            glPopMatrix();
        }

        if (animation.isHurt || animation.isDead)
        {
            glPopAttrib();
        }
    }

    glPopMatrix();

    // Render active butterfly heal sequence
    if (animation.isHealing)
    {
        glPushMatrix();

        // Apply uniformScale to perfectly shrink the path offsets along with the character model
        const float HAND_OFFSET_X = -20.0f * uniformScale;
        const float HAND_OFFSET_Y = 15.0f * uniformScale;
        const float HAND_OFFSET_Z = -15.0f * uniformScale;

        // Apply 2D rotation matrix based on horizontal facing direction
        float hx = (HAND_OFFSET_X * std::cos(facingYaw)) + (HAND_OFFSET_Z * std::sin(facingYaw));
        float hz = (-HAND_OFFSET_X * std::sin(facingYaw)) + (HAND_OFFSET_Z * std::cos(facingYaw));

        float handWorldX = posX + hx;
        float handWorldY = posY + 18.7f + HAND_OFFSET_Y;
        float handWorldZ = posZ + hz;

        const float BUCKET_HEIGHT_OFFSET = 25.0f * uniformScale;
        float bucketWorldX = posX;
        float bucketWorldY = posY + 18.7f + BUCKET_HEIGHT_OFFSET;
        float bucketWorldZ = posZ;

        float flyX = handWorldX, flyY = handWorldY, flyZ = handWorldZ;
        float scale = 1.0f;
        float flapAngle = 0.0f;

        if (animation.healTimer <= 0.5f)
        {
            flapAngle = std::sin(animation.healTimer * 20.0f) * 30.0f;
        }
        else if (animation.healTimer <= 1.5f)
        {
            float t = (animation.healTimer - 0.5f) / 1.0f;
            flyX = handWorldX + (bucketWorldX - handWorldX) * t;
            flyY = handWorldY + (bucketWorldY - handWorldY) * t;
            flyZ = handWorldZ + (bucketWorldZ - handWorldZ) * t;
            flapAngle = std::sin(animation.healTimer * 20.0f) * 30.0f;
        }
        else
        {
            flyX = bucketWorldX;
            flyY = bucketWorldY;
            flyZ = bucketWorldZ;
            flapAngle = 0.0f;
            scale = 1.0f - ((animation.healTimer - 1.5f) / 0.5f);
            if (scale < 0.0f) scale = 0.0f;
        }

        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);

        glPushMatrix();
        glTranslatef(flyX, flyY, flyZ);
        glScalef(scale, scale, scale);

        glPushMatrix();
        glScalef(0.3f, 0.3f, 0.3f);
        ::myvirtualworld.butterfly.draw(flapAngle);
        glPopMatrix();
        glPopMatrix();

        // Render cross particles orbiting around Kinger's head/bucket
        if (animation.healTimer >= 1.5f)
        {
            glPushMatrix();
            glTranslatef(bucketWorldX, bucketWorldY, bucketWorldZ);

            glColor3f(0.2f, 1.0f, 0.2f);

            float crossTimer = animation.healTimer - 1.5f;

            for (int i = 0; i < 3; i++)
            {
                glPushMatrix();

                float offsetTimer = crossTimer + (i * 0.1f);
                float height = offsetTimer * 15.0f;
                float orbitX = std::sin(offsetTimer * 10.0f + i) * 3.0f;
                float orbitZ = std::cos(offsetTimer * 10.0f + i) * 3.0f;

                glTranslatef(orbitX, height, orbitZ);
                glRotatef(offsetTimer * 300.0f, 0.0f, 1.0f, 0.0f);

                float crossScale = 1.0f - (height / 10.0f);
                if (crossScale < 0.0f) crossScale = 0.0f;
                crossScale *= 0.7f;
                glScalef(crossScale, crossScale, crossScale);

                glPushMatrix();
                glScalef(0.6f, 2.0f, 0.6f);
                drawSolidCubeFallback(1.0f);
                glPopMatrix();

                glPushMatrix();
                glScalef(2.0f, 0.6f, 0.6f);
                drawSolidCubeFallback(1.0f);
                glPopMatrix();

                glPopMatrix();
            }

            glPopMatrix();
        }

        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Render gun bullet particles slightly offset from model center
    glTranslatef(0.0f, 3.0f, 0.0f);
    drawBullet();
    drawMuzzleParticles();
}
