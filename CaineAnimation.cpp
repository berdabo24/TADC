#include <GL/glut.h>
#include <cmath>
#include "CaineAnimation.hpp"

// Animation Configuration Constants
static const float HOVER_SPEED = 2.0f;           // Frequency of the vertical bobbing animation
static const float HOVER_AMPLITUDE = 1.2f;       // Vertical distance range for the bobbing motion
static const float TILT_AMPLITUDE = 3.0f;        // Maximum tilt angle in degrees for breathing sway
static const float JAW_SPEED = 4.0f;             // Speed frequency of the mouth breathing movement
static const float JAW_MAX_OPEN = 25.0f;         // Maximum angle for the jaw flap animation
static const float STAFF_SWAY_AMPLITUDE = 10.0f;  // Maximum sway angle for Caine's staff

/**
 * Constructor that resets all timing counters, flags, and animation factors to their defaults.
 */
CaineAnimation::CaineAnimation()
{
    idleTimer = 0.0f;
    hoverOffset = 0.0f;
    bodyTiltAngle = 0.0f;
    jawFlapAngle = 0.0f;
    staffSwayAngle = 0.0f;
    mouthOpenFactor = 0.0f;
    isShootingState = false;
    shootingTimer = 0.0f;
    isLayingDown = false;
    layDownFactor = 0.0f;
    isLeaningForward = false;
    leanForwardFactor = 0.0f;
    isDead = false;
    deathTimer = 0.0f;
    isLaughing = false;
    isHurt = false;
    hurtTimer = 0.0f;
}

/**
 * Updates idle animation states based on elapsed frame delta time.
 * dt Delta time since the last frame (in seconds).
 * 
 * Computes hover offsets, body tilt angles, and mouth opening factors via harmonic sine wave motions.
 */
void CaineAnimation::updateIdleState(float dt)
{
    idleTimer += dt;
    
    // Smooth harmonic vertical bobbing
    hoverOffset = std::sin(idleTimer * HOVER_SPEED) * HOVER_AMPLITUDE;
    
    // Subtle breathing roll tilt (using half frequency to prevent locking with bobbing)
    bodyTiltAngle = std::sin(idleTimer * HOVER_SPEED * 0.5f) * TILT_AMPLITUDE;
    
    if (isLaughing)
    {
        // Laughing mouth flap: very fast and wide (4x standard speed)
        mouthOpenFactor = (std::sin(idleTimer * JAW_SPEED * 4.0f) * 0.5f) + 0.5f;
    }
    else
    {
        // Continuous smooth sine wave for TMJ mouth open factor (ranges [0.0, 1.0])
        mouthOpenFactor = (std::sin(idleTimer * JAW_SPEED) * 0.5f) + 0.5f;
    }
    
    // Calculate final rotation angle from factor
    jawFlapAngle = mouthOpenFactor * JAW_MAX_OPEN;
}

/**
 * Triggers the shooting state machine sequence, resetting the progress timer.
 */
void CaineAnimation::triggerShootingState()
{
    isShootingState = true;
    shootingTimer = 0.0f;
}

/**
 * Steps the shooting animation timeline by delta time.
 * deltaTime Elapsed frame time (in seconds).
 */
void CaineAnimation::updateShootingState(float deltaTime)
{
    if (isShootingState)
    {
        shootingTimer += deltaTime;

        // Cap animation at 100% progress and reset active trigger flag
        if (shootingTimer >= 1.0f)
        {
            shootingTimer = 1.0f;
            isShootingState = false;
        }
    }
}

/**
 * Applies the multi-stage hand-pointing and shooting rotations to the OpenGL matrix stack.
 * 
 * Performs 3 distinct phases based on progress time (0.0 to 1.0):
 * - Phase 1 (0.0s to 0.3s): Hand raises up and rolls back (roll goes -30 to -70 deg).
 * - Phase 2 (0.3s to 0.7s): Hand swings forward to aim (pitch goes 180 to 70 deg).
 * - Phase 3 (0.7s to 1.0s): Hand returns smoothly back to resting posture.
 */
void CaineAnimation::applyShootingAnimation() const
{
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = -30.0f * (1.0f - layDownFactor); // Fades initial resting roll to 0 when laying down

    if (isShootingState || shootingTimer > 0.0f) 
    {
        float t = shootingTimer;

        if (t <= 0.3f)
        {
            // Stage 1: Raise/roll hand up
            float progress = t / 0.3f;
            float factor = std::sin(progress * 3.14159265358979323846f / 2.0f);
            pitch = 180.0f * factor;
            yaw = 0.0f;
            roll = -30.0f + (-40.0f * factor);
        }
        else if (t <= 0.7f)
        {
            // Stage 2: Swing forward to point
            float progress = (t - 0.3f) / 0.4f;
            float factor = std::sin(progress * 3.14159265358979323846f / 2.0f);
            pitch = 180.0f - 110.0f * factor; 
            yaw = 0.0f;
            roll = -70.0f;
        }
        else if (t <= 1.0f)
        {
            // Stage 3: Return smoothly to rest
            float progress = (t - 0.7f) / 0.3f;
            float factor = std::sin(progress * 3.14159265358979323846f / 2.0f);
            pitch = 70.0f * (1.0f - factor);
            yaw = 0.0f;
            roll = -70.0f + (40.0f * factor);
        }
    }

    // Apply accumulated transformations to the matrix stack
    glRotatef(pitch, 1.0f, 0.0f, 0.0f);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    glRotatef(roll, 0.0f, 0.0f, 1.0f);
}

/**
 * Toggles whether Caine is laying down.
 */
void CaineAnimation::toggleLayDown()
{
    isLayingDown = !isLayingDown;
}

/**
 * Interpolates the lay down transition factor.
 * deltaTime Elapsed frame time (in seconds).
 */
void CaineAnimation::updateLayDown(float deltaTime)
{
    if (isLayingDown)
    {
        layDownFactor += 3.0f * deltaTime;
        if (layDownFactor > 1.0f)
            layDownFactor = 1.0f;
    }
    else
    {
        layDownFactor -= 3.0f * deltaTime;
        if (layDownFactor < 0.0f)
            layDownFactor = 0.0f;
    }
}

/**
 * Toggles whether Caine is leaning forward.
 */
void CaineAnimation::toggleLeanForward()
{
    isLeaningForward = !isLeaningForward;
}

/**
 * Interpolates the lean forward transition factor.
 * deltaTime Elapsed frame time (in seconds).
 */
void CaineAnimation::updateLeanForward(float deltaTime)
{
    if (isLeaningForward)
    {
        leanForwardFactor += 3.0f * deltaTime;
        if (leanForwardFactor > 1.0f)
            leanForwardFactor = 1.0f;
    }
    else
    {
        leanForwardFactor -= 3.0f * deltaTime;
        if (leanForwardFactor < 0.0f)
            leanForwardFactor = 0.0f;
    }
}

/**
 * Updates the damage stun flashing timer.
 * deltaTime The elapsed frame time in seconds.
 */
void CaineAnimation::updateHurtState(float deltaTime)
{
    if (!isHurt) return;

    hurtTimer += deltaTime;
    if (hurtTimer >= 0.5f)
    {
        isHurt = false;
        hurtTimer = 0.0f;
    }
}

/**
 * Activates the hurt state, interrupting active animations.
 */
void CaineAnimation::triggerHurt()
{
    isHurt = true;
    hurtTimer = 0.0f;

    // Immediately stop ongoing shooting
    isShootingState = false;
    shootingTimer = 0.0f;
}
