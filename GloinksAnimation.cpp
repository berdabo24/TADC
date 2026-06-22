#include <GL/glut.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include "GloinksAnimation.hpp"
#include "CNAWorld.hpp"

extern ProjectWorld::MyVirtualWorld myvirtualworld;
extern int difficultyLevel;

using namespace ProjectGloinks;

/**
 * Constructor that clears the collection of active Gloinks.
 */
GloinksAnimation::GloinksAnimation()
{
    activeGloinks.clear();
    spawnTimer = 0.0f;
    spawnInterval = 2.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 1.0f)); // random 2 to 3 seconds
    maxGloinks = 10;
}

/**
 * Spawns the initial set of 6 Gloinks spaced out along the X axis.
 */
void GloinksAnimation::initGloinks()
{
    activeGloinks.clear();
    spawnTimer = 0.0f;
    spawnInterval = 2.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 1.0f));

    if (::myvirtualworld.isDebugMode)
    {
        const float groundLevel = -18.7f;
        const float spacing = 15.0f;

        for (int i = 0; i < 6; ++i)
        {
            Gloink gloink;
            gloink.posX = (i - 2.5f) * spacing;
            gloink.posZ = 0.0f;
            gloink.posY = groundLevel;
            gloink.groundLevel = groundLevel;
            gloink.jumpTimer = static_cast<float>(std::rand() % 100);
            gloink.shapeType = i;
            gloink.isHurt = false;
            gloink.hurtTimer = 0.0f;
            gloink.health = 5; // Debug ones have 5 health
            gloink.isDead = false;
            gloink.deathTimer = 0.0f;
            gloink.isKnockedBack = false;
            gloink.knockbackTimer = 0.0f;
            gloink.knockbackDirX = 0.0f;
            gloink.knockbackDirZ = 0.0f;
            gloink.isPaused = false;
            gloink.pauseTimer = 0.0f;

            activeGloinks.push_back(gloink);
        }
    }
}

void GloinksAnimation::spawnGloink()
{
    Gloink gloink;
    // Spawn at random coordinates within boundaries (-280 to 280)
    gloink.posX = -280.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 560.0f));
    gloink.posZ = -280.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 560.0f));
    gloink.posY = -18.7f;
    gloink.groundLevel = -18.7f;
    gloink.jumpTimer = static_cast<float>(std::rand() % 100);
    gloink.shapeType = std::rand() % 6; // random shape type 0 to 5
    gloink.isHurt = false;
    gloink.hurtTimer = 0.0f;
    gloink.health = 1; // gameplay Gloinks have 1 health
    gloink.isDead = false;
    gloink.deathTimer = 0.0f;
    gloink.isKnockedBack = false;
    gloink.knockbackTimer = 0.0f;
    gloink.knockbackDirX = 0.0f;
    gloink.knockbackDirZ = 0.0f;
    gloink.isPaused = false;
    gloink.pauseTimer = 0.0f;

    activeGloinks.push_back(gloink);

    // Spawn Caine's digital teleport poof particles centered at the Gloink's base on the ground
    ::myvirtualworld.caine.spawnTeleportPoof(gloink.posX, gloink.posY - 11.0f, gloink.posZ);
}

/**
 * Updates positions and timers for all active Gloinks.
 * deltaTime The elapsed frame time in seconds.
 */
void GloinksAnimation::updateGloinks(float deltaTime)
{
    const float bounceSpeed = 3.0f;
    const float bounceHeight = 5.0f;
    const float groundLevel = -18.7f;
    const float hurtDuration = 0.5f;
    const float deathDuration = 1.0f; // 1 second death animation
    const float movementSpeed = 25.0f;
    const float knockbackSpeed = 100.0f; // smooth glide back speed

    // Spawning: only in gameplay mode (not debug mode)
    if (!::myvirtualworld.isDebugMode)
    {
        if (::myvirtualworld.isCaineActive)
        {
            if (::myvirtualworld.caine.animation.isDead)
            {
                activeGloinks.clear();
                maxGloinks = 0;
            }
            else
            {
                if (difficultyLevel == 2)
                {
                    if (::myvirtualworld.caine.currentPhase == 1)
                    {
                        maxGloinks = 5;
                    }
                    else if (::myvirtualworld.caine.currentPhase == 2)
                    {
                        maxGloinks = 10;
                    }
                    else if (::myvirtualworld.caine.currentPhase == 3)
                    {
                        maxGloinks = 15;
                    }
                }
                else
                {
                    if (::myvirtualworld.caine.currentPhase == 1)
                    {
                        maxGloinks = 0;
                        activeGloinks.clear();
                    }
                    else if (::myvirtualworld.caine.currentPhase == 2)
                    {
                        maxGloinks = 5;
                    }
                    else if (::myvirtualworld.caine.currentPhase == 3)
                    {
                        maxGloinks = 10;
                    }
                }
            }
        }

        if (maxGloinks > 0)
        {
            int aliveCount = 0;
            for (const auto& g : activeGloinks)
            {
                if (!g.isDead) aliveCount++;
            }

            if (aliveCount < maxGloinks)
            {
                spawnTimer += deltaTime;
                if (spawnTimer >= spawnInterval)
                {
                    spawnGloink();
                    spawnTimer = 0.0f;
                    spawnInterval = 2.0f + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / 1.0f));
                }
            }
        }
    }

    // Update active gloinks
    for (auto it = activeGloinks.begin(); it != activeGloinks.end(); )
    {
        if (it->isDead)
        {
            it->deathTimer += deltaTime;
            it->posY = it->groundLevel; // Keep it on the ground when dead
            
            if (it->deathTimer >= deathDuration)
            {
                // In debug mode, respawn in place
                if (::myvirtualworld.isDebugMode)
                {
                    it->isDead = false;
                    it->deathTimer = 0.0f;
                    it->health = 5;
                    it->isHurt = false;
                    it->hurtTimer = 0.0f;
                    ++it;
                }
                else
                {
                    it = activeGloinks.erase(it);
                }
            }
            else
            {
                ++it;
            }
        }
        else
        {
            // Bouncing update
            it->jumpTimer += deltaTime;
            it->posY = it->groundLevel + std::abs(std::sin(it->jumpTimer * bounceSpeed)) * bounceHeight;

            // Hurt state update
            if (it->isHurt)
            {
                it->hurtTimer += deltaTime;
                if (it->hurtTimer >= hurtDuration)
                {
                    it->isHurt = false;
                    it->hurtTimer = 0.0f;
                }
            }

            // seeking, knockback, pause, and collision: gameplay only (not in debug mode)
            if (!::myvirtualworld.isDebugMode)
            {
                // Mutual separation from other active Gloinks (to prevent bunching up)
                for (size_t otherIdx = 0; otherIdx < activeGloinks.size(); ++otherIdx)
                {
                    auto& other = activeGloinks[otherIdx];
                    if (&other == &(*it) || other.isDead) continue;

                    float sepDx = it->posX - other.posX;
                    float sepDz = it->posZ - other.posZ;
                    float sepDist = std::sqrt(sepDx * sepDx + sepDz * sepDz);
                    float sepMinDist = 9.0f * ::myvirtualworld.gloinks.uniformScale;

                    if (sepDist < sepMinDist)
                    {
                        if (sepDist < 0.01f)
                        {
                            // Push randomly if exactly overlapping
                            it->posX += (std::rand() % 10 - 5) * 0.5f;
                            it->posZ += (std::rand() % 10 - 5) * 0.5f;
                        }
                        else
                        {
                            float overlap = sepMinDist - sepDist;
                            it->posX += (sepDx / sepDist) * overlap * 0.5f;
                            it->posZ += (sepDz / sepDist) * overlap * 0.5f;
                        }
                        it->posX = std::max(-280.0f, std::min(280.0f, it->posX));
                        it->posZ = std::max(-280.0f, std::min(280.0f, it->posZ));
                    }
                }

                // Handle state machine: KnockedBack, Paused, or seeking
                if (it->isKnockedBack)
                {
                    // Glide backwards away from player
                    it->posX += it->knockbackDirX * knockbackSpeed * deltaTime;
                    it->posZ += it->knockbackDirZ * knockbackSpeed * deltaTime;
                    
                    // Clamp inside bounds
                    it->posX = std::max(-280.0f, std::min(280.0f, it->posX));
                    it->posZ = std::max(-280.0f, std::min(280.0f, it->posZ));

                    it->knockbackTimer -= deltaTime;
                    if (it->knockbackTimer <= 0.0f)
                    {
                        it->isKnockedBack = false;
                        it->isPaused = true;
                        it->pauseTimer = 1.0f; // pause for 1 second
                    }
                }
                else if (it->isPaused)
                {
                    // Remain stationary
                    it->pauseTimer -= deltaTime;
                    if (it->pauseTimer <= 0.0f)
                    {
                        it->isPaused = false;
                    }
                }
                else
                {
                    // Normal seeking towards player (Kinger)
                    float pkX = ::myvirtualworld.kinger.posX;
                    float pkZ = ::myvirtualworld.kinger.posZ;
                    float dx = pkX - it->posX;
                    float dz = pkZ - it->posZ;
                    float dist = std::sqrt(dx * dx + dz * dz);

                    if (dist > 0.1f)
                    {
                        float dirX = dx / dist;
                        float dirZ = dz / dist;
                        it->posX += dirX * movementSpeed * deltaTime;
                        it->posZ += dirZ * movementSpeed * deltaTime;
                        
                        it->posX = std::max(-280.0f, std::min(280.0f, it->posX));
                        it->posZ = std::max(-280.0f, std::min(280.0f, it->posZ));
                    }

                    // Collision check against player
                    float kingerScale = ::myvirtualworld.kinger.uniformScale;
                    float gloinkScale = ::myvirtualworld.gloinks.uniformScale;
                    float collisionThreshold = 3.0f * kingerScale + 4.5f * gloinkScale;

                    if (dist <= collisionThreshold)
                    {
                        // Kinger takes damage
                        if (!::myvirtualworld.kinger.animation.isDead)
                        {
                            ::myvirtualworld.kinger.takeDamage(1);
                        }

                        // Trigger knockback state (0.5 seconds duration, speed is 100.0f/s)
                        float pushX, pushZ;
                        if (dist > 0.01f)
                        {
                            pushX = -(dx / dist);
                            pushZ = -(dz / dist);
                        }
                        else
                        {
                            pushX = 1.0f;
                            pushZ = 0.0f;
                        }

                        it->isKnockedBack = true;
                        it->knockbackTimer = 0.5f; // glide back for 0.5s (totaling 50.0 units distance)
                        it->knockbackDirX = pushX;
                        it->knockbackDirZ = pushZ;
                        it->isPaused = false;
                        it->pauseTimer = 0.0f;
                    }
                }

                // Resolve dynamic and static obstacle collisions in the environment (cubes, pillars, etc.)
                // Allow them to phase through outer boundaries/walls by not calling checkWallCollision.
                float resolvedX = it->posX;
                float resolvedZ = it->posZ;
                float resolvedGroundY = -18.7f;
                float gloinkRad = 4.5f * ::myvirtualworld.gloinks.uniformScale;
                ::myvirtualworld.environment.checkObstacleCollision(
                    it->posX, it->posZ, it->posY, gloinkRad,
                    resolvedX, resolvedZ, resolvedGroundY,
                    true // isGloink = true
                );
                it->posX = resolvedX;
                it->posZ = resolvedZ;

                // Smoothly interpolate groundLevel towards the resolved ground height
                float diffY = resolvedGroundY - it->groundLevel;
                const float climbSpeed = 40.0f; // vertical units per second
                if (std::abs(diffY) > 0.001f)
                {
                    float step = climbSpeed * deltaTime;
                    if (std::abs(diffY) <= step)
                    {
                        it->groundLevel = resolvedGroundY;
                    }
                    else
                    {
                        it->groundLevel += (diffY > 0.0f ? 1.0f : -1.0f) * step;
                    }
                }
                else
                {
                    it->groundLevel = resolvedGroundY;
                }
            }

            ++it;
        }
    }
}

/**
 * Damages a specific Gloink by index, triggering a hurt state or death.
 * index The index of the Gloink to damage.
 */
void GloinksAnimation::hurtGloink(int index)
{
    if (index >= 0 && index < (int)activeGloinks.size())
    {
        Gloink& gloink = activeGloinks[index];
        if (gloink.isDead)
            return; // Already dead

        gloink.health -= 1;
        if (gloink.health <= 0)
        {
            gloink.health = 0;
            gloink.isDead = true;
            gloink.deathTimer = 0.0f;
        }
        else
        {
            gloink.isHurt = true;
            gloink.hurtTimer = 0.0f;
        }
    }
}

/**
 * Applies rotation, scaling, and fall translation animations to the active OpenGL matrix.
 * center The bounding box visual center of the shape model.
 * isDead Whether the Gloink is dead.
 * deathTimer The current progression timer of the death animation.
 * isHurt Whether the Gloink is hurt.
 * hurtTimer The current progression timer of the hurt animation.
 */
void GloinksAnimation::applyAnimationTransforms(Vec3 center, bool isDead, float deathTimer, bool isHurt, float hurtTimer) const
{
    // Apply stiff fall to side (Minecraft style) if dead
    if (isDead)
    {
        float progress = deathTimer / 1.0f; // deathDuration = 1.0s
        if (progress > 1.0f) progress = 1.0f;
        float fallAngle = progress * 90.0f; // 0 to 90 degrees
        
        // --- Pivot Matrix Sandwich for Falling ---
        // 1. Translate bottom center of shape to origin
        glTranslatef(0.0f, -center.y, 0.0f);
        // 2. Rotate to drop sideways
        glRotatef(fallAngle, 0.0f, 0.0f, 1.0f);
        // 3. Translate back
        glTranslatef(0.0f, center.y, 0.0f);
    }

    // Apply hurt visual effect (squash & stretch from bottom pivot)
    if (!isDead && isHurt)
    {
        float hurtIntensity = 1.0f - (hurtTimer / 0.5f);
        if (hurtIntensity < 0.0f) hurtIntensity = 0.0f;

        // --- Pivot Matrix Sandwich for Squash and Stretch ---
        // 1. Translate bottom center of shape to origin
        glTranslatef(0.0f, -center.y, 0.0f);
        // 2. Apply scale distortion
        glScalef(1.0f + 0.3f * hurtIntensity, 1.0f - 0.5f * hurtIntensity, 1.0f + 0.3f * hurtIntensity);
        // 3. Translate back
        glTranslatef(0.0f, center.y, 0.0f);
    }
}

/**
 * Applies coloring/material properties based on the Gloink's current health state.
 * isDead Whether the Gloink is dead.
 * isHurt Whether the Gloink is hurt.
 * hurtTimer The current progression timer of the hurt animation.
 */
void GloinksAnimation::applyColorAndMaterial(bool isDead, bool isHurt, float hurtTimer) const
{
    if (isDead)
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
    else if (isHurt)
    {
        float hurtIntensity = 1.0f - (hurtTimer / 0.5f);
        if (hurtIntensity < 0.0f) hurtIntensity = 0.0f;

        // Flash red
        glColor3f(1.0f, 1.0f - hurtIntensity, 1.0f - hurtIntensity);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
}

/**
 * Applies rotation on the Z-axis for rolling/spinning motion during jumping.
 * jumpTimer The jump timer value of the Gloink.
 * isDead Whether the Gloink is dead.
 */
void GloinksAnimation::applyRotation(float jumpTimer, bool isDead) const
{
    float spinSpeed = 90.0f;
    if (!isDead)
    {
        glRotatef(jumpTimer * spinSpeed, 0.0f, 0.0f, 1.0f);
    }
}

/**
 * Restores normal material and lighting attributes.
 * isDead Whether the Gloink was dead.
 */
void GloinksAnimation::cleanupColorAndMaterial(bool isDead) const
{
    if (isDead)
    {
        glPopAttrib();
    }
}
