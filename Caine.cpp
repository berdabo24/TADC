#include <GL/glut.h>
#include <iostream>
#include "Caine.hpp"
#include <cmath>    
#include "CNAWorld.hpp"

extern ProjectWorld::MyVirtualWorld myvirtualworld;
extern bool showHitboxes;
extern bool isTestArena;
extern int caineDeathSeqState;
extern float caineDeathSeqTimer;
extern int caineIntroSeqState;
extern int difficultyLevel;

using namespace ProjectCaine;

// Adjustable settings for Caine's constant float height
static const float PLAYER_GROUND_Y = -18.7f;
static const float PLAYER_MODEL_HEIGHT = 22.0f; // Visual height offset parameter for Kinger's model
static const float CAINE_MODEL_Y_OFFSET = 19.5f; // Caine's model parts are translated down by -19.5f internally in Caine's drawing code
static const float CAINE_HOVER_HEIGHT_Y = PLAYER_GROUND_Y + 2.0f * PLAYER_MODEL_HEIGHT + CAINE_MODEL_Y_OFFSET; // -18.7f + 44.0f + 19.5f = 44.8f

// Compensating position shift when Caine is in LayDown state (tilted/rolled)
static const float CAINE_LAYDOWN_SHIFT_X = 15.0f;
static const float CAINE_LAYDOWN_SHIFT_Y = 8.0f;

static void drawSolidSphereFallback(float radius, int slices, int stacks)
{
    float dtheta = 3.14159265f / stacks;
    float dphi = 2.0f * 3.14159265f / slices;

    for (int i = 0; i < stacks; i++)
    {
        float theta1 = i * dtheta;
        float theta2 = (i + 1) * dtheta;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= slices; j++)
        {
            float phi = j * dphi;

            float x1 = std::sin(theta1) * std::cos(phi);
            float y1 = std::cos(theta1);
            float z1 = std::sin(theta1) * std::sin(phi);

            float x2 = std::sin(theta2) * std::cos(phi);
            float y2 = std::cos(theta2);
            float z2 = std::sin(theta2) * std::sin(phi);

            glNormal3f(x1, y1, z1);
            glVertex3f(x1 * radius, y1 * radius, z1 * radius);

            glNormal3f(x2, y2, z2);
            glVertex3f(x2 * radius, y2 * radius, z2 * radius);
        }
        glEnd();
    }
}

/**
 * Construct a new Caine object and initialize all loading flags, positions, scales, and spawn state.
 */
Caine::Caine(bool isClone)
    : isClone(isClone)
{
    hatLoaded = false;
    leftHandLoaded = false;
    leftLegLoaded = false;
    leftPalmLoaded = false;
    lowerJawLoaded = false;
    rightHandLoaded = false;
    rightLegLoaded = false;
    rightPalmLoaded = false;
    staffLoaded = false;
    tongueLoaded = false;
    tursoLoaded = false;
    upperJawLoaded = false;
    leftEyeLoaded = false;
    rightEyeLoaded = false;
    uniformScale = 1.0f;
    posX = 0.0f;
    posY = 0.0f;
    posZ = 0.0f;
    spawnX = 0.0f;
    spawnY = 0.0f;
    spawnZ = 0.0f;
    spawnPositionSaved = false;

    // Initialize AI variables
    aiFlightTimer = 0.0f;
    aiFlightDuration = 3.0f;
    aiTargetX = 0.0f;
    aiTargetZ = -120.0f;
    aiTeleportTimer = 0.0f;
    aiTeleportInterval = 10.0f;
    isTeleporting = false;
    isAppearing = false;
    teleportTransitionTimer = 0.0f;
    visualScaleFactor = 1.0f;
    facingYaw = 0.0f;
    maxHealth = 100;
    currentHealth = maxHealth;
    currentPhase = 1;
    isTransitioning = false;
    transitionTimer = 0.0f;
    transitionDuration = 8.0f;

    shootCooldownTimer = 0.0f;
    shootInterval = 1.0f;
    for (int i = 0; i < MAX_CAINE_PROJECTILES; i++)
    {
        projectiles[i].active = false;
        projectiles[i].posX = 0.0f;
        projectiles[i].posY = 0.0f;
        projectiles[i].posZ = 0.0f;
        projectiles[i].dirX = 0.0f;
        projectiles[i].dirY = 0.0f;
        projectiles[i].dirZ = 0.0f;
        projectiles[i].lifeTimer = 0.0f;
    }
    for (int i = 0; i < MAX_TELEPORT_PARTICLES; i++)
    {
        teleportParticles[i].active = false;
        teleportParticles[i].posX = 0.0f;
        teleportParticles[i].posY = 0.0f;
        teleportParticles[i].posZ = 0.0f;
        teleportParticles[i].velX = 0.0f;
        teleportParticles[i].velY = 0.0f;
        teleportParticles[i].velZ = 0.0f;
        teleportParticles[i].size = 0.0f;
        teleportParticles[i].r = 0.0f;
        teleportParticles[i].g = 0.0f;
        teleportParticles[i].b = 0.0f;
        teleportParticles[i].alpha = 0.0f;
        teleportParticles[i].lifeTime = 0.0f;
        teleportParticles[i].maxLife = 0.0f;
    }

    sweepActive = false;
    sweepDirection = 0;
    nextSweepDirection = 0;
    sweepCurrentPos = 0.0f;
    sweepSpeed = 150.0f;
    sweepTimer = 0.0f;
    sweepInterval = 2.0f;
    wasLayingDown = false;
    testArenaSweepMode = false;

    sweepActive2 = false;
    sweepDirection2 = 0;
    nextSweepDirection2 = 0;
    sweepCurrentPos2 = 0.0f;

    sweepActive3 = false;
    sweepDirection3 = 0;
    nextSweepDirection3 = 0;
    sweepCurrentPos3 = 0.0f;

    sweepActive4 = false;
    sweepDirection4 = 0;
    nextSweepDirection4 = 0;
    sweepCurrentPos4 = 0.0f;

    doctorStrangeState = 0;
    doctorStrangeTimer = 0.0f;
    particleSpawnTimer = 0.0f;

    if (!isClone)
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            clones[i] = new Caine(true);
        }
    }
    else
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            clones[i] = nullptr;
        }
    }
}

Caine::~Caine()
{
    if (!isClone)
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            delete clones[i];
            clones[i] = nullptr;
        }
    }
}

void Caine::copyModelDataFrom(const Caine& other)
{
    hatModel = other.hatModel;
    leftHandModel = other.leftHandModel;
    leftLegModel = other.leftLegModel;
    leftPalmModel = other.leftPalmModel;
    lowerJawModel = other.lowerJawModel;
    rightHandModel = other.rightHandModel;
    rightLegModel = other.rightLegModel;
    rightPalmModel = other.rightPalmModel;
    staffModel = other.staffModel;
    tongueModel = other.tongueModel;
    tursoModel = other.tursoModel;
    upperJawModel = other.upperJawModel;
    leftEyeModel = other.leftEyeModel;
    rightEyeModel = other.rightEyeModel;

    hatLoaded = other.hatLoaded;
    leftHandLoaded = other.leftHandLoaded;
    leftLegLoaded = other.leftLegLoaded;
    leftPalmLoaded = other.leftPalmLoaded;
    lowerJawLoaded = other.lowerJawLoaded;
    rightHandLoaded = other.rightHandLoaded;
    rightLegLoaded = other.rightLegLoaded;
    rightPalmLoaded = other.rightPalmLoaded;
    staffLoaded = other.staffLoaded;
    tongueLoaded = other.tongueLoaded;
    tursoLoaded = other.tursoLoaded;
    upperJawLoaded = other.upperJawLoaded;
    leftEyeLoaded = other.leftEyeLoaded;
    rightEyeLoaded = other.rightEyeLoaded;

    hatTextureID = other.hatTextureID;
    leftHandTextureID = other.leftHandTextureID;
    leftLegTextureID = other.leftLegTextureID;
    leftPalmTextureID = other.leftPalmTextureID;
    lowerJawTextureID = other.lowerJawTextureID;
    rightHandTextureID = other.rightHandTextureID;
    rightLegTextureID = other.rightLegTextureID;
    rightPalmTextureID = other.rightPalmTextureID;
    staffTextureID = other.staffTextureID;
    tongueTextureID = other.tongueTextureID;
    tursoTextureID = other.tursoTextureID;
    upperJawTextureID = other.upperJawTextureID;
    leftEyeTextureID = other.leftEyeTextureID;
    rightEyeTextureID = other.rightEyeTextureID;

    uniformScale = other.uniformScale;
}

void Caine::resetAI()
{
    aiFlightTimer = 0.0f;
    aiFlightDuration = 3.0f;
    aiTargetX = 0.0f;
    aiTargetZ = -120.0f;
    aiTeleportTimer = 0.0f;
    aiTeleportInterval = 8.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4.0f));
    isTeleporting = false;
    isAppearing = false;
    teleportTransitionTimer = 0.0f;
    visualScaleFactor = 1.0f;
    facingYaw = 0.0f;
    testArenaSweepMode = false;
    doctorStrangeState = 0;
    doctorStrangeTimer = 0.0f;
    particleSpawnTimer = 0.0f;

    // Reset Caine position to spawn
    if (spawnPositionSaved)
    {
        posX = spawnX;
        posY = spawnY;
        posZ = spawnZ;
    }
    else
    {
        posX = 0.0f;
        posY = 0.0f;
        posZ = -120.0f;
    }

    // Reset animations
    animation.isLaughing = false;
    animation.isLayingDown = false;
    animation.isLeaningForward = false;
    animation.isShootingState = false;

    if (difficultyLevel == 2)
    {
        maxHealth = 200;
    }
    else
    {
        maxHealth = 100;
    }
    currentHealth = maxHealth;
    currentPhase = 1;
    isTransitioning = false;
    transitionTimer = 0.0f;

    shootCooldownTimer = 0.0f;
    for (int i = 0; i < MAX_CAINE_PROJECTILES; i++)
    {
        projectiles[i].active = false;
    }
    for (int i = 0; i < MAX_TELEPORT_PARTICLES; i++)
    {
        teleportParticles[i].active = false;
    }

    sweepActive = false;
    sweepTimer = 0.0f;
    sweepInterval = 2.0f;
    nextSweepDirection = rand() % 4;
    wasLayingDown = false;

    sweepActive2 = false;
    sweepDirection2 = 0;
    nextSweepDirection2 = rand() % 4;
    sweepCurrentPos2 = 0.0f;

    sweepActive3 = false;
    sweepDirection3 = 0;
    nextSweepDirection3 = rand() % 4;
    sweepCurrentPos3 = 0.0f;

    sweepActive4 = false;
    sweepDirection4 = 0;
    nextSweepDirection4 = rand() % 4;
    sweepCurrentPos4 = 0.0f;

    if (!isClone)
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            if (clones[i])
            {
                clones[i]->resetAI();
            }
        }
    }
}

/**
 * Update Caine's animation state variables and check death timeline/respawn.
 * deltaTime Elapsed frame time (in seconds).
 */
void Caine::update(float deltaTime)
{
    // Capture the initial spawn position on the first update pass
    if (!spawnPositionSaved)
    {
        spawnX = posX;
        spawnY = posY;
        spawnZ = posZ;
        spawnPositionSaved = true;
    }

    if (caineIntroSeqState > 0)
    {
        posX = 0.0f;
        posY = 0.0f;
        posZ = -120.0f;

        animation.hoverOffset = 0.0f;
        animation.bodyTiltAngle = 0.0f;
        animation.jawFlapAngle = 0.0f;
        animation.staffSwayAngle = 0.0f;
        animation.mouthOpenFactor = 0.0f;
        animation.isShootingState = false;
        animation.shootingTimer = 0.0f;
        animation.isLayingDown = false;
        animation.layDownFactor = 0.0f;
        animation.isLeaningForward = false;
        animation.leanForwardFactor = 0.0f;
        animation.isDead = false;
        animation.deathTimer = 0.0f;
        animation.isLaughing = false;
        animation.isHurt = false;
        animation.hurtTimer = 0.0f;
        
        isTeleporting = false;
        isAppearing = false;
        visualScaleFactor = 1.0f;
        sweepActive = false;
        return;
    }

    if (difficultyLevel == 2)
    {
        if (currentPhase == 1 || currentPhase == 2)
        {
            shootInterval = 0.2f;
        }
        else if (currentPhase == 3)
        {
            shootInterval = 2.0f; // Greatly decreased
        }
    }
    else
    {
        shootInterval = 1.0f;
    }

    if (caineDeathSeqState > 0)
    {
        animation.hoverOffset = 0.0f;
        animation.bodyTiltAngle = 0.0f;
        animation.jawFlapAngle = 0.0f;
        animation.staffSwayAngle = 0.0f;
        animation.mouthOpenFactor = 0.0f;
        animation.isShootingState = false;
        animation.shootingTimer = 0.0f;
        animation.isLayingDown = false;
        animation.layDownFactor = 0.0f;
        animation.isLeaningForward = false;
        animation.leanForwardFactor = 0.0f;
        animation.isDead = false;
        animation.deathTimer = 0.0f;
        animation.isLaughing = false;
        animation.isHurt = false;
        animation.hurtTimer = 0.0f;
        
        isTeleporting = false;
        isAppearing = false;
        visualScaleFactor = 1.0f;
        sweepActive = false;
        
        return;
    }

    if (isClone)
    {
        testArenaSweepMode = false;
        sweepActive = false;
        animation.isLayingDown = false;
        animation.isLaughing = false;
        isTransitioning = false;
    }
    else if (isTestArena || doctorStrangeState > 0)
    {
        if (doctorStrangeState == 1)
        {
            doctorStrangeTimer += deltaTime;
            
            particleSpawnTimer += deltaTime;
            if (particleSpawnTimer >= 0.1f)
            {
                particleSpawnTimer = 0.0f;
                spawnTeleportPoof(posX, posY, posZ);
            }

            if (doctorStrangeTimer >= 3.0f)
            {
                for (int i = 0; i < MAX_TELEPORT_PARTICLES; i++)
                {
                    teleportParticles[i].active = false;
                }

                float spotsX[MAX_CLONES + 1];
                float spotsZ[MAX_CLONES + 1];
                float spotsY[MAX_CLONES + 1];
                for (int j = 0; j < MAX_CLONES + 1; j++)
                {
                    spotsX[j] = -100.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 200.0f);
                    spotsZ[j] = -180.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 230.0f);
                    spotsY[j] = CAINE_HOVER_HEIGHT_Y + (-5.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 10.0f));
                }

                int realSpot = rand() % (MAX_CLONES + 1);
                posX = spotsX[realSpot];
                posY = spotsY[realSpot];
                posZ = spotsZ[realSpot];
                spawnTeleportPoof(posX, posY, posZ);

                int cloneIdx = 0;
                for (int j = 0; j < MAX_CLONES + 1; j++)
                {
                    if (j == realSpot) continue;
                    Caine* clone = clones[cloneIdx++];
                    if (clone)
                    {
                        clone->posX = spotsX[j];
                        clone->posY = spotsY[j];
                        clone->posZ = spotsZ[j];
                        clone->testArenaSweepMode = false;
                        clone->sweepActive = false;
                        clone->animation.isLayingDown = false;
                        clone->animation.isLaughing = false;
                        clone->animation.isDead = false;
                        clone->animation.isHurt = false;
                        clone->currentHealth = clone->maxHealth;
                        clone->aiTeleportTimer = 0.0f;
                        clone->currentPhase = currentPhase;
                        clone->aiTargetX = -120.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 240.0f));
                        clone->aiTargetZ = -200.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 300.0f));
                        clone->visualScaleFactor = 1.0f;
                        clone->facingYaw = facingYaw;
                        
                        for (int k = 0; k < MAX_TELEPORT_PARTICLES; k++)
                        {
                            clone->teleportParticles[k].active = false;
                        }
                        clone->spawnTeleportPoof(clone->posX, clone->posY, clone->posZ);
                    }
                }

                // Reveal real Caine at his new position
                visualScaleFactor = 1.0f;
                doctorStrangeState = 2;
                doctorStrangeTimer = 0.0f;
            }
        }
        else if (doctorStrangeState == 2)
        {
            for (int i = 0; i < MAX_CLONES; i++)
            {
                if (clones[i])
                {
                    clones[i]->update(deltaTime);
                }
            }
        }
        else if (doctorStrangeState == 3)
        {
            doctorStrangeTimer += deltaTime;
            if (doctorStrangeTimer >= 5.0f)
            {
                doctorStrangeState = 0;
                doctorStrangeTimer = 0.0f;
                // Hide all clones
                for (int i = 0; i < MAX_CLONES; i++)
                {
                    if (clones[i])
                    {
                        clones[i]->visualScaleFactor = 0.0f;
                    }
                }
                animation.isLeaningForward = false;
                animation.leanForwardFactor = 0.0f;
            }
        }
    }

    // Accumulate death timer if Caine is dead
    if (animation.isDead)
    {
        animation.deathTimer += deltaTime;
        // Automatically respawn/reanimate Caine after exactly 2 seconds
        if (animation.deathTimer >= 2.0f)
        {
            if (::myvirtualworld.isDebugMode)
            {
                animation.isDead = false;
                animation.deathTimer = 0.0f;
                currentHealth = maxHealth;
                currentPhase = 1;
                isTransitioning = false;

                if (spawnPositionSaved)
                {
                    posX = spawnX;
                    posY = spawnY;
                    posZ = spawnZ;
                }
            }
        }
    }

    // Forward updates to sub-animations
    animation.updateIdleState(deltaTime);
    animation.updateShootingState(deltaTime);
    animation.updateLayDown(deltaTime);
    animation.updateLeanForward(deltaTime);
    animation.updateHurtState(deltaTime);

    if (!::myvirtualworld.isDebugMode && !animation.isDead)
    {
        if (isTransitioning)
        {
            transitionTimer += deltaTime;
            if (transitionTimer >= transitionDuration)
            {
                isTransitioning = false;
                transitionTimer = 0.0f;
                currentHealth = maxHealth;
                animation.isLeaningForward = false;
                aiTeleportTimer = 0.0f;
                shootCooldownTimer = 0.0f;
            }
            else
            {
                // Heal slowly over the last 5 seconds (from 3.0s to 8.0s)
                if (transitionTimer > 3.0f)
                {
                    float t_heal = (transitionTimer - 3.0f) / 5.0f;
                    if (t_heal > 1.0f) t_heal = 1.0f;
                    currentHealth = (int)(t_heal * maxHealth);

                    // Spawn healing particles floats upwards
                    int spawnedThisFrame = 0;
                    for (int j = 0; j < MAX_TELEPORT_PARTICLES && spawnedThisFrame < 2; j++)
                    {
                        if (!teleportParticles[j].active)
                        {
                            teleportParticles[j].active = true;
                            // Spawn offset in a cylinder around Caine
                            float radius = 5.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;
                            float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;
                            teleportParticles[j].posX = posX + radius * std::cos(angle);
                            teleportParticles[j].posY = posY - 22.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 15.0f;
                            teleportParticles[j].posZ = posZ + radius * std::sin(angle);

                            // Float upwards slowly
                            teleportParticles[j].velX = -5.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;
                            teleportParticles[j].velY = 15.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 15.0f;
                            teleportParticles[j].velZ = -5.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f;

                            // Color matches the active phase health bar color!
                            if (currentPhase == 2) // Phase 2: Orange/Yellow
                            {
                                if (rand() % 2 == 0) {
                                    teleportParticles[j].r = 1.0f; teleportParticles[j].g = 0.5f; teleportParticles[j].b = 0.0f;
                                } else {
                                    teleportParticles[j].r = 1.0f; teleportParticles[j].g = 0.8f; teleportParticles[j].b = 0.0f;
                                }
                            }
                            else if (currentPhase == 3) // Phase 3: Crimson/Red
                            {
                                if (rand() % 2 == 0) {
                                    teleportParticles[j].r = 1.0f; teleportParticles[j].g = 0.0f; teleportParticles[j].b = 0.1f;
                                } else {
                                    teleportParticles[j].r = 0.8f; teleportParticles[j].g = 0.1f; teleportParticles[j].b = 0.2f;
                                }
                            }
                            else
                            {
                                teleportParticles[j].r = 1.0f; teleportParticles[j].g = 1.0f; teleportParticles[j].b = 1.0f;
                            }

                            teleportParticles[j].size = 1.5f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.0f;
                            teleportParticles[j].alpha = 1.0f;
                            teleportParticles[j].lifeTime = 0.0f;
                            teleportParticles[j].maxLife = 0.6f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.4f;

                            spawnedThisFrame++;
                        }
                    }
                }
                else
                {
                    currentHealth = 0;
                }
            }
        }
        else if (!isClone && (doctorStrangeState == 1 || doctorStrangeState == 3))
        {
            if (doctorStrangeState == 3)
            {
                animation.isLeaningForward = true;
                animation.leanForwardFactor = 1.0f;
                animation.isLayingDown = false;
                animation.layDownFactor = 0.0f;
                animation.isLaughing = false;
            }
        }
        else
        {
            // 1. Maintain a constant height level of 2x the player model's height above the ground plane
            float targetY = CAINE_HOVER_HEIGHT_Y;
        posY += (targetY - posY) * 3.0f * deltaTime;

        // 2. Face the player at all times
        float dx = ::myvirtualworld.kinger.posX - posX;
        float dz = ::myvirtualworld.kinger.posZ - posZ;
        facingYaw = std::atan2(dx, dz) * 57.2957795f; // convert to degrees

        // 3. Handle Teleportation and Flight Behavior
        if (isClone || (!isTeleporting && !isAppearing))
        {
            if (!isClone)
            {
                // Update teleport cooldown
                aiTeleportTimer += deltaTime;
                if (aiTeleportTimer >= aiTeleportInterval)
                {
                    isTeleporting = true;
                    teleportTransitionTimer = 0.0f;
                }
            }

            // Normal flight behavior
            aiFlightTimer += deltaTime;
            if (aiFlightTimer >= aiFlightDuration)
            {
                // Safe arena bounds: X [-120.0, 120.0], Z [-200.0, 100.0]
                aiTargetX = -120.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 240.0f));
                aiTargetZ = -200.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 300.0f));
                aiFlightTimer = 0.0f;
                aiFlightDuration = 2.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 3.0f));
            }

            // Move slowly towards target
            float fdx = aiTargetX - posX;
            float fdz = aiTargetZ - posZ;
            float dist = std::sqrt(fdx * fdx + fdz * fdz);
            if (dist > 1.0f)
            {
                float flySpeed = 15.0f; // Slow fly speed
                posX += (fdx / dist) * flySpeed * deltaTime;
                posZ += (fdz / dist) * flySpeed * deltaTime;
            }

            // 4. Handle shooting at the player (only in default state, not in LayDown or LeaningForward)
            // Also suppress real Caine's shooting during Doctor Strange state 2
            if (!animation.isLayingDown && !animation.isLeaningForward && (isClone || doctorStrangeState != 2))
            {
                shootCooldownTimer += deltaTime;
                if (shootCooldownTimer >= shootInterval)
                {
                    shootCooldownTimer = 0.0f;
                    animation.triggerShootingState();
                    spawnProjectile();
                }
            }
            else
            {
                shootCooldownTimer = 0.0f;
            }
        }
        else if (isTeleporting)
        {
            // Shrink phase: shrink from scale 1.0 to 0.0 in 0.4 seconds
            teleportTransitionTimer += deltaTime;
            float progress = teleportTransitionTimer / 0.4f;
            if (progress >= 1.0f)
            {
                progress = 1.0f;
                isTeleporting = false;
                
                // Spawn poof at old position before moving Caine
                spawnTeleportPoof(posX, posY, posZ);
                
                // Teleport to a random location in the arena
                posX = -120.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 240.0f));
                posZ = -200.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 300.0f));
                
                // Face player immediately
                float ndx = ::myvirtualworld.kinger.posX - posX;
                float ndz = ::myvirtualworld.kinger.posZ - posZ;
                facingYaw = std::atan2(ndx, ndz) * 57.2957795f;

                // Pick random pose: default or laughing state (not leaning state)
                animation.isLeaningForward = false;
                animation.isShootingState = false;
                
                bool shouldTriggerDS = false;
                if (!isClone && !isTestArena)
                {
                    if (difficultyLevel == 2)
                    {
                        if (currentPhase == 1)
                        {
                            shouldTriggerDS = (rand() % 5 == 0);
                        }
                        else if (currentPhase == 2 || currentPhase == 3)
                        {
                            shouldTriggerDS = (rand() % 3 == 0);
                        }
                    }
                    else
                    {
                        if (currentPhase == 2 || currentPhase == 3)
                        {
                            shouldTriggerDS = (rand() % 5 == 0);
                        }
                    }
                }

                if (shouldTriggerDS)
                {
                    doctorStrangeState = 1;
                    doctorStrangeTimer = 0.0f;
                    particleSpawnTimer = 0.0f;
                    visualScaleFactor = 0.0f;
                    
                    for (int k = 0; k < MAX_CAINE_PROJECTILES; k++)
                    {
                        projectiles[k].active = false;
                    }
                    aiTeleportTimer = 0.0f;
                    if (difficultyLevel == 2 && (currentPhase == 2 || currentPhase == 3))
                    {
                        aiTeleportInterval = 4.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f));
                    }
                    else
                    {
                        aiTeleportInterval = 8.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4.0f));
                    }
                }
                else
                {
                    if (isTestArena)
                    {
                        if (testArenaSweepMode)
                        {
                            animation.isLaughing = true;
                            animation.isLayingDown = true;
                        }
                        else
                        {
                            animation.isLaughing = false;
                            animation.isLayingDown = false;
                        }
                    }
                    else if (rand() % 2 == 0)
                    {
                        animation.isLaughing = true;
                        animation.isLayingDown = true;
                    }
                    else
                    {
                        animation.isLaughing = false;
                        animation.isLayingDown = false;
                    }

                    // Start appear phase
                    isAppearing = true;
                    teleportTransitionTimer = 0.0f;
                    
                    // Spawn poof at new position after placing Caine
                    spawnTeleportPoof(posX, posY, posZ);
                    
                    // Pick next random teleport cooldown interval
                    aiTeleportTimer = 0.0f;
                    if (difficultyLevel == 2 && (currentPhase == 2 || currentPhase == 3))
                    {
                        aiTeleportInterval = 4.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 2.0f)); // 4.0 to 6.0 seconds
                    }
                    else
                    {
                        aiTeleportInterval = 8.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 4.0f)); // 8.0 to 12.0 seconds
                    }
                }
            }
            visualScaleFactor = 1.0f - progress;
        }
        else if (isAppearing)
        {
            // Appear phase: grow from scale 0.0 to 1.0 in 0.4 seconds
            teleportTransitionTimer += deltaTime;
            float progress = teleportTransitionTimer / 0.4f;
            if (progress >= 1.0f)
            {
                progress = 1.0f;
                isAppearing = false;
            }
            visualScaleFactor = progress;
        }

        // 5. Handle sweep attack (only in LayDown state)
        // Detect transition into LayDown state to set initial 2-second delay
        if (animation.isLayingDown && !wasLayingDown)
        {
            sweepTimer = 0.0f;
            sweepInterval = 2.0f; // first attack spawns after 2 seconds
            int directions[4] = {0, 1, 2, 3};
            for (int i = 3; i > 0; --i)
            {
                int j = rand() % (i + 1);
                std::swap(directions[i], directions[j]);
            }
            nextSweepDirection = directions[0];
            nextSweepDirection2 = directions[1];
            nextSweepDirection3 = directions[2];
            nextSweepDirection4 = directions[3];
        }
        wasLayingDown = animation.isLayingDown;

        if (animation.isLayingDown)
        {
            bool anySweepActive = sweepActive || sweepActive2 || sweepActive3 || sweepActive4;
            if (!anySweepActive)
            {
                sweepTimer += deltaTime;
                if (sweepTimer >= sweepInterval)
                {
                    sweepActive = true;
                    sweepTimer = 0.0f;

                    // Use the pre-rolled direction (shown in the HUD warning)
                    sweepDirection = nextSweepDirection;

                    // Choose start coordinate
                    if (sweepDirection == 0 || sweepDirection == 2)
                    {
                        sweepCurrentPos = -290.0f;
                    }
                    else
                    {
                        sweepCurrentPos = 290.0f;
                    }

                    int numSweeps = 1;
                    if (difficultyLevel == 2)
                    {
                        if (currentPhase == 1) numSweeps = 2;
                        else if (currentPhase == 2) numSweeps = 3;
                        else if (currentPhase == 3) numSweeps = 4;
                    }
                    else
                    {
                        numSweeps = (currentPhase == 3) ? 2 : 1;
                    }

                    if (numSweeps >= 2)
                    {
                        sweepActive2 = true;
                        sweepDirection2 = nextSweepDirection2;
                        sweepCurrentPos2 = (sweepDirection2 == 0 || sweepDirection2 == 2) ? -290.0f : 290.0f;
                    }
                    else
                    {
                        sweepActive2 = false;
                    }

                    if (numSweeps >= 3)
                    {
                        sweepActive3 = true;
                        sweepDirection3 = nextSweepDirection3;
                        sweepCurrentPos3 = (sweepDirection3 == 0 || sweepDirection3 == 2) ? -290.0f : 290.0f;
                    }
                    else
                    {
                        sweepActive3 = false;
                    }

                    if (numSweeps >= 4)
                    {
                        sweepActive4 = true;
                        sweepDirection4 = nextSweepDirection4;
                        sweepCurrentPos4 = (sweepDirection4 == 0 || sweepDirection4 == 2) ? -290.0f : 290.0f;
                    }
                    else
                    {
                        sweepActive4 = false;
                    }

                    // Set next cooldown interval and pre-roll the NEXT sweep direction
                    sweepInterval = 2.0f;
                    int directions[4] = {0, 1, 2, 3};
                    for (int i = 3; i > 0; --i)
                    {
                        int j = rand() % (i + 1);
                        std::swap(directions[i], directions[j]);
                    }
                    nextSweepDirection = directions[0];
                    nextSweepDirection2 = directions[1];
                    nextSweepDirection3 = directions[2];
                    nextSweepDirection4 = directions[3];
                }
            }
        }
        else
        {
            sweepTimer = 0.0f;
        }

        // Update active sweep movement and collision checking
        if (sweepActive)
        {
            if (sweepDirection == 0 || sweepDirection == 2)
            {
                sweepCurrentPos += sweepSpeed * deltaTime;
                if (sweepCurrentPos >= 290.0f)
                {
                    sweepActive = false;
                }
            }
            else
            {
                sweepCurrentPos -= sweepSpeed * deltaTime;
                if (sweepCurrentPos <= -290.0f)
                {
                    sweepActive = false;
                }
            }

            // Check collision against Kinger
            if (sweepActive && !::myvirtualworld.kinger.animation.isDead)
            {
                bool hit = false;
                float playerX = ::myvirtualworld.kinger.posX;
                float playerY = ::myvirtualworld.kinger.posY;
                float playerZ = ::myvirtualworld.kinger.posZ;
                
                float wallTopY = -18.7f + 12.0f; // wall sits on ground (-18.7) and has total height of 12.0 (panelHalfHeight is 6.0)
                float hitThickness = 8.0f; // horizontal collision width of the sweeping plane

                if (sweepDirection == 0 || sweepDirection == 1) // North-to-South or South-to-North
                {
                    if (std::abs(playerZ - sweepCurrentPos) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }
                else // East-to-West or West-to-East
                {
                    if (std::abs(playerX - sweepCurrentPos) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }

                if (hit)
                {
                    int damage = 4;
                    if (difficultyLevel == 2) damage = 5;
                    else if (currentPhase == 2) damage = 5;
                    else if (currentPhase == 3) damage = 5;

                    ::myvirtualworld.kinger.takeDamage(damage); 
                }
            }
        }

        // Update active sweep 2 movement and collision checking
        if (sweepActive2)
        {
            if (sweepDirection2 == 0 || sweepDirection2 == 2)
            {
                sweepCurrentPos2 += sweepSpeed * deltaTime;
                if (sweepCurrentPos2 >= 290.0f)
                {
                    sweepActive2 = false;
                }
            }
            else
            {
                sweepCurrentPos2 -= sweepSpeed * deltaTime;
                if (sweepCurrentPos2 <= -290.0f)
                {
                    sweepActive2 = false;
                }
            }

            // Check collision against Kinger
            if (sweepActive2 && !::myvirtualworld.kinger.animation.isDead)
            {
                bool hit = false;
                float playerX = ::myvirtualworld.kinger.posX;
                float playerY = ::myvirtualworld.kinger.posY;
                float playerZ = ::myvirtualworld.kinger.posZ;
                
                float wallTopY = -18.7f + 12.0f; 
                float hitThickness = 8.0f; 

                if (sweepDirection2 == 0 || sweepDirection2 == 1) // North-to-South or South-to-North
                {
                    if (std::abs(playerZ - sweepCurrentPos2) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }
                else // East-to-West or West-to-East
                {
                    if (std::abs(playerX - sweepCurrentPos2) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }

                if (hit)
                {
                    int damage = 5; // Sweep 2, 3, and 4 deal 5 damage
                    ::myvirtualworld.kinger.takeDamage(damage);
                }
            }
        }

        // Update active sweep 3 movement and collision checking
        if (sweepActive3)
        {
            if (sweepDirection3 == 0 || sweepDirection3 == 2)
            {
                sweepCurrentPos3 += sweepSpeed * deltaTime;
                if (sweepCurrentPos3 >= 290.0f)
                {
                    sweepActive3 = false;
                }
            }
            else
            {
                sweepCurrentPos3 -= sweepSpeed * deltaTime;
                if (sweepCurrentPos3 <= -290.0f)
                {
                    sweepActive3 = false;
                }
            }

            // Check collision against Kinger
            if (sweepActive3 && !::myvirtualworld.kinger.animation.isDead)
            {
                bool hit = false;
                float playerX = ::myvirtualworld.kinger.posX;
                float playerY = ::myvirtualworld.kinger.posY;
                float playerZ = ::myvirtualworld.kinger.posZ;
                
                float wallTopY = -18.7f + 12.0f; 
                float hitThickness = 8.0f; 

                if (sweepDirection3 == 0 || sweepDirection3 == 1)
                {
                    if (std::abs(playerZ - sweepCurrentPos3) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }
                else
                {
                    if (std::abs(playerX - sweepCurrentPos3) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }

                if (hit)
                {
                    int damage = 5;
                    ::myvirtualworld.kinger.takeDamage(damage);
                }
            }
        }

        // Update active sweep 4 movement and collision checking
        if (sweepActive4)
        {
            if (sweepDirection4 == 0 || sweepDirection4 == 2)
            {
                sweepCurrentPos4 += sweepSpeed * deltaTime;
                if (sweepCurrentPos4 >= 290.0f)
                {
                    sweepActive4 = false;
                }
            }
            else
            {
                sweepCurrentPos4 -= sweepSpeed * deltaTime;
                if (sweepCurrentPos4 <= -290.0f)
                {
                    sweepActive4 = false;
                }
            }

            // Check collision against Kinger
            if (sweepActive4 && !::myvirtualworld.kinger.animation.isDead)
            {
                bool hit = false;
                float playerX = ::myvirtualworld.kinger.posX;
                float playerY = ::myvirtualworld.kinger.posY;
                float playerZ = ::myvirtualworld.kinger.posZ;
                
                float wallTopY = -18.7f + 12.0f; 
                float hitThickness = 8.0f; 

                if (sweepDirection4 == 0 || sweepDirection4 == 1)
                {
                    if (std::abs(playerZ - sweepCurrentPos4) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }
                else
                {
                    if (std::abs(playerX - sweepCurrentPos4) <= hitThickness && playerY < wallTopY)
                    {
                        hit = true;
                    }
                }

                if (hit)
                {
                    int damage = 5;
                    ::myvirtualworld.kinger.takeDamage(damage);
                }
            }
        }
        }
    }
    else
    {
        visualScaleFactor = 1.0f;
    }

    // Update active projectiles (unconditional)
    float projectileSpeed = 150.0f;
    for (int i = 0; i < MAX_CAINE_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            projectiles[i].posX += projectiles[i].dirX * projectileSpeed * deltaTime;
            projectiles[i].posY += projectiles[i].dirY * projectileSpeed * deltaTime;
            projectiles[i].posZ += projectiles[i].dirZ * projectileSpeed * deltaTime;
            projectiles[i].lifeTimer += deltaTime;

            // Deactivate if lifetime exceeded
            if (projectiles[i].lifeTimer >= 3.0f)
            {
                projectiles[i].active = false;
                continue;
            }

            // Check collision against Kinger
            float kingerScale = ::myvirtualworld.kinger.uniformScale;
            float radius = 3.0f * kingerScale;
            float hitTolerance = 4.0f; // tolerance for thin line collision check

            float dx = projectiles[i].posX - ::myvirtualworld.kinger.posX;
            float dz = projectiles[i].posZ - ::myvirtualworld.kinger.posZ;
            float distXZ = std::sqrt(dx * dx + dz * dz);

            if (distXZ <= (radius + hitTolerance))
            {
                float kingerMinY = ::myvirtualworld.kinger.posY;
                float kingerMaxY = ::myvirtualworld.kinger.posY + 22.0f * kingerScale;

                if (projectiles[i].posY >= (kingerMinY - hitTolerance) && 
                    projectiles[i].posY <= (kingerMaxY + hitTolerance))
                {
                    // Hit! Apply damage to Kinger based on currentPhase
                    int damage = 1;
                    if (difficultyLevel == 2)
                    {
                        if (currentPhase == 3) damage = 5;
                        else damage = 2;
                    }
                    else if (currentPhase == 2 || currentPhase == 3)
                    {
                        damage = 2;
                    }
                    ::myvirtualworld.kinger.takeDamage(damage);
                    projectiles[i].active = false;
                }
            }
        }
    }

    // Update active teleport particles (unconditional)
    for (int i = 0; i < MAX_TELEPORT_PARTICLES; i++)
    {
        if (teleportParticles[i].active)
        {
            teleportParticles[i].posX += teleportParticles[i].velX * deltaTime;
            teleportParticles[i].posY += teleportParticles[i].velY * deltaTime;
            teleportParticles[i].posZ += teleportParticles[i].velZ * deltaTime;
            
            // Add a bit of air resistance/drag
            teleportParticles[i].velX *= 0.92f;
            teleportParticles[i].velY *= 0.92f;
            teleportParticles[i].velZ *= 0.92f;
            
            teleportParticles[i].lifeTime += deltaTime;
            if (teleportParticles[i].lifeTime >= teleportParticles[i].maxLife)
            {
                teleportParticles[i].active = false;
            }
            else
            {
                // Fade out alpha
                float progress = teleportParticles[i].lifeTime / teleportParticles[i].maxLife;
                teleportParticles[i].alpha = 1.0f - progress;
                
                // Grow size slightly as they expand, then shrink
                if (progress < 0.2f)
                {
                    teleportParticles[i].size += deltaTime * 8.0f;
                }
                else
                {
                    teleportParticles[i].size -= deltaTime * 4.0f;
                    if (teleportParticles[i].size < 0.1f) teleportParticles[i].size = 0.1f;
                }
            }
        }
    }
}

/**
 * Handle death initialization, resetting all active posture modifiers and snapping to spawn position.
 */
void Caine::triggerDeath()
{
    animation.isDead = true;
    animation.deathTimer = 0.0f;
    animation.isLaughing = false;
    animation.isLayingDown = false;
    animation.layDownFactor = 0.0f;
    animation.isLeaningForward = false;
    animation.leanForwardFactor = 0.0f;
    animation.isShootingState = false;
    animation.shootingTimer = 0.0f;
    animation.isHurt = false;
    animation.hurtTimer = 0.0f;
}

/**
 * Handle hurt initialization, resetting active shooting postures and starting the flashing timer.
 */
void Caine::triggerHurt()
{
    animation.triggerHurt();
}

/**
 * Set the uniform scale multiplier of the model.
 * scale Uniform scale factor.
 */
void Caine::setScale(float scale)
{
    uniformScale = scale;
}

void Caine::takeDamage(int amount)
{
    if (isClone)
    {
        triggerHurt();
        return;
    }

    if (!isClone && doctorStrangeState == 2)
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            if (clones[i])
            {
                spawnTeleportPoof(clones[i]->posX, clones[i]->posY, clones[i]->posZ);
                for (int k = 0; k < MAX_CAINE_PROJECTILES; k++)
                {
                    clones[i]->projectiles[k].active = false;
                }
            }
        }
        doctorStrangeState = 3;
        doctorStrangeTimer = 0.0f;
        animation.isLeaningForward = true;
        animation.leanForwardFactor = 1.0f;
        animation.isLayingDown = false;
        animation.layDownFactor = 0.0f;
        animation.isLaughing = false;
        sweepActive = false;
        triggerHurt();
        return;
    }

    if (caineDeathSeqState > 0 || caineIntroSeqState > 0) return;
    if (isTransitioning) return; // Invincible during transition state!
    if (animation.isHurt || animation.isDead) return;

    currentHealth -= amount;
    if (currentHealth <= 0)
    {
        currentHealth = 0;
        if (isTestArena)
        {
            currentHealth = maxHealth;
            animation.isHurt = false;
            animation.hurtTimer = 0.0f;
        }
        else if (currentPhase < 3)
        {
            currentPhase++;
            isTransitioning = true;
            transitionTimer = 0.0f;

            // Set max HP for the new phase (all phases share the same pool)
            if (difficultyLevel == 2)
            {
                if (currentPhase == 2) maxHealth = 250;
                else if (currentPhase == 3) maxHealth = 300;
            }
            else
            {
                if (currentPhase == 2) maxHealth = 150;
                else if (currentPhase == 3) maxHealth = 200;
            }

            // Clear other animation/AI states to lean forward cleanly
            animation.isLeaningForward = true;
            animation.isLayingDown = false;
            animation.layDownFactor = 0.0f;
            animation.isShootingState = false;
            animation.shootingTimer = 0.0f;
            animation.isLaughing = false;
            isTeleporting = false;
            isAppearing = false;
            visualScaleFactor = 1.0f;
            sweepActive = false;
            sweepActive2 = false;
            sweepActive3 = false;
            sweepActive4 = false;

            if (difficultyLevel == 2)
            {
                // Clear projectiles
                for (int k = 0; k < MAX_CAINE_PROJECTILES; k++)
                {
                    projectiles[k].active = false;
                }
                for (int cIdx = 0; cIdx < MAX_CLONES; cIdx++)
                {
                    if (clones[cIdx])
                    {
                        for (int k = 0; k < MAX_CAINE_PROJECTILES; k++)
                        {
                            clones[cIdx]->projectiles[k].active = false;
                        }
                    }
                }

                // Clear Gloinks
                ::myvirtualworld.gloinks.animation.activeGloinks.clear();

                // Clear active falling meteors
                for (int m = 0; m < ProjectEnvironment::Environment::NUM_METEORS; m++)
                {
                    ::myvirtualworld.environment.meteors[m].state = 0;
                }
                for (int p = 0; p < ProjectEnvironment::Environment::MAX_METEOR_PARTICLES; p++)
                {
                    ::myvirtualworld.environment.meteorParticles[p].active = false;
                }

                // Clear player's active bullet tracer
                ::myvirtualworld.kinger.animation.isBulletActive = false;

                // Grant +1 heal charge (capped at 3)
                if (::myvirtualworld.kinger.animation.butterflyCharges < 3)
                {
                    ::myvirtualworld.kinger.animation.butterflyCharges++;
                }
            }

            doctorStrangeState = 0;
            doctorStrangeTimer = 0.0f;
            for (int i = 0; i < MAX_CLONES; i++)
            {
                if (clones[i])
                {
                    clones[i]->sweepActive = false;
                    clones[i]->sweepActive2 = false;
                    clones[i]->sweepActive3 = false;
                    clones[i]->sweepActive4 = false;
                    for (int k = 0; k < MAX_CAINE_PROJECTILES; k++)
                    {
                        clones[i]->projectiles[k].active = false;
                    }
                }
            }

            if (currentPhase == 3 || (difficultyLevel == 2 && currentPhase == 2))
            {
                ::myvirtualworld.environment.isMeteorModeActive = true;
            }
        }
        else
        {
            if (!isTestArena)
            {
                caineDeathSeqState = 1;
                caineDeathSeqTimer = 0.0f;
                myvirtualworld.audioManager.stopBackgroundMusic();

                // Reset Caine states
                animation.isLayingDown = false;
                animation.layDownFactor = 0.0f;
                animation.isShootingState = false;
                animation.shootingTimer = 0.0f;
                animation.isLaughing = false;
                isTeleporting = false;
                isAppearing = false;
                visualScaleFactor = 1.0f;
                sweepActive = false;
            }
            else
            {
                triggerDeath();
            }
        }
    }
    else
    {
        triggerHurt();
    }
}

Vec3 Caine::getCaineWorldCenter() const
{
    float currentScale = uniformScale * visualScaleFactor;
    float layDownAngleRad = animation.layDownFactor * 70.0f * 0.0174532925f;
    
    // Rotation shift of the scaled center around the pivot (0, -19.5)
    float rotShiftX = -11.0f * currentScale * std::sin(layDownAngleRad);
    float rotShiftY = 11.0f * currentScale * (std::cos(layDownAngleRad) - 1.0f);
    
    // Translation shifts (applied before scaling in draw())
    float transShiftX = animation.layDownFactor * CAINE_LAYDOWN_SHIFT_X;
    float transShiftY = animation.layDownFactor * (20.0f + CAINE_LAYDOWN_SHIFT_Y);
    
    // Total local shifts
    float localShiftX = transShiftX + rotShiftX;
    float localShiftY = transShiftY + rotShiftY;
    
    // Center Y starts at posY + hoverOffset, plus local Y shift, plus the baseline offset (-19.5 + 11) scaled
    float centerY = posY + animation.hoverOffset + localShiftY - 19.5f + 11.0f * currentScale;
    
    // Rotate localShiftX by facingYaw (in radians) to project to world coordinates
    float yawRad = facingYaw * 0.0174532925f;
    float centerX = posX + localShiftX * std::cos(yawRad);
    float centerZ = posZ - localShiftX * std::sin(yawRad);

    Vec3 center;
    center.x = centerX;
    center.y = centerY;
    center.z = centerZ;
    return center;
}

void Caine::spawnProjectile()
{
    for (int i = 0; i < MAX_CAINE_PROJECTILES; i++)
    {
        if (!projectiles[i].active)
        {
            projectiles[i].active = true;
            projectiles[i].lifeTimer = 0.0f;

            // Spawn at Caine's center
            Vec3 spawnPos = getCaineWorldCenter();
            projectiles[i].posX = spawnPos.x;
            projectiles[i].posY = spawnPos.y;
            projectiles[i].posZ = spawnPos.z;

            // Direction vector pointing to the player's chest (approx. posY + 11.0f)
            float kingerScale = ::myvirtualworld.kinger.uniformScale;
            float targetX = ::myvirtualworld.kinger.posX;
            float targetY = ::myvirtualworld.kinger.posY + 11.0f * kingerScale;
            float targetZ = ::myvirtualworld.kinger.posZ;

            float dx = targetX - projectiles[i].posX;
            float dy = targetY - projectiles[i].posY;
            float dz = targetZ - projectiles[i].posZ;

            float length = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (length > 0.1f)
            {
                projectiles[i].dirX = dx / length;
                projectiles[i].dirY = dy / length;
                projectiles[i].dirZ = dz / length;
            }
            else
            {
                projectiles[i].dirX = 0.0f;
                projectiles[i].dirY = -1.0f;
                projectiles[i].dirZ = 0.0f;
            }
            break;
        }
    }
}

void Caine::spawnTeleportPoof(float x, float y, float z)
{
    int spawned = 0;
    for (int i = 0; i < MAX_TELEPORT_PARTICLES && spawned < 20; i++)
    {
        if (!teleportParticles[i].active)
        {
            teleportParticles[i].active = true;
            teleportParticles[i].posX = x;
            // Center the poof vertically on Caine
            teleportParticles[i].posY = y + 11.0f; 
            teleportParticles[i].posZ = z;
            
            // Random direction in a sphere
            float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;
            float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.14159265f;
            float speed = 20.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 30.0f; // 20 to 50 units/sec
            
            teleportParticles[i].velX = speed * std::sin(phi) * std::cos(theta);
            teleportParticles[i].velY = speed * std::cos(phi);
            teleportParticles[i].velZ = speed * std::sin(phi) * std::sin(theta);
            
            // Glitch colors (cyan, magenta, yellow, white, red)
            int colType = rand() % 5;
            if (colType == 0) { // Cyan
                teleportParticles[i].r = 0.0f; teleportParticles[i].g = 0.9f; teleportParticles[i].b = 1.0f;
            } else if (colType == 1) { // Magenta / Pink
                teleportParticles[i].r = 1.0f; teleportParticles[i].g = 0.1f; teleportParticles[i].b = 0.6f;
            } else if (colType == 2) { // Yellow
                teleportParticles[i].r = 1.0f; teleportParticles[i].g = 0.9f; teleportParticles[i].b = 0.0f;
            } else if (colType == 3) { // Red
                teleportParticles[i].r = 1.0f; teleportParticles[i].g = 0.1f; teleportParticles[i].b = 0.1f;
            } else { // White
                teleportParticles[i].r = 1.0f; teleportParticles[i].g = 1.0f; teleportParticles[i].b = 1.0f;
            }
            
            teleportParticles[i].size = 2.0f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f; // size 2 to 6
            teleportParticles[i].alpha = 1.0f;
            teleportParticles[i].lifeTime = 0.0f;
            teleportParticles[i].maxLife = 0.5f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.3f; // 0.5 to 0.8 seconds
            
            spawned++;
        }
    }
}


bool Caine::loadHat(const std::string& filePath)
{
    hatLoaded = hatModel.loadFromObjText(filePath);
    return hatLoaded;
}

bool Caine::loadLeftHand(const std::string& filePath)
{
    leftHandLoaded = leftHandModel.loadFromObjText(filePath);
    return leftHandLoaded;
}

bool Caine::loadLeftLeg(const std::string& filePath)
{
    leftLegLoaded = leftLegModel.loadFromObjText(filePath);
    return leftLegLoaded;
}

bool Caine::loadLeftPalm(const std::string& filePath)
{
    leftPalmLoaded = leftPalmModel.loadFromObjText(filePath);
    return leftPalmLoaded;
}

bool Caine::loadLowerJaw(const std::string& filePath)
{
    lowerJawLoaded = lowerJawModel.loadFromObjText(filePath);
    return lowerJawLoaded;
}

bool Caine::loadRightHand(const std::string& filePath)
{
    rightHandLoaded = rightHandModel.loadFromObjText(filePath);
    return rightHandLoaded;
}

bool Caine::loadRightLeg(const std::string& filePath)
{
    rightLegLoaded = rightLegModel.loadFromObjText(filePath);
    return rightLegLoaded;
}

bool Caine::loadRightPalm(const std::string& filePath)
{
    rightPalmLoaded = rightPalmModel.loadFromObjText(filePath);
    return rightPalmLoaded;
}

bool Caine::loadStaff(const std::string& filePath)
{
    staffLoaded = staffModel.loadFromObjText(filePath);
    return staffLoaded;
}

bool Caine::loadTongue(const std::string& filePath)
{
    tongueLoaded = tongueModel.loadFromObjText(filePath);
    return tongueLoaded;
}

bool Caine::loadTurso(const std::string& filePath)
{
    tursoLoaded = tursoModel.loadFromObjText(filePath);
    return tursoLoaded;
}

bool Caine::loadUpperJaw(const std::string& filePath)
{
    upperJawLoaded = upperJawModel.loadFromObjText(filePath);
    return upperJawLoaded;
}

bool Caine::loadRightEye(const std::string& filePath)
{
    rightEyeLoaded = rightEyeModel.loadFromObjText(filePath);
    return rightEyeLoaded;
}

bool Caine::loadLeftEye(const std::string& filePath)
{
    leftEyeLoaded = leftEyeModel.loadFromObjText(filePath);
    return leftEyeLoaded;
}

// ==========================================
// Rendering Methods for Individual Parts
// ==========================================

/**
 * Render Caine's hat.
 */
void Caine::drawHat() const
{
    if (!hatLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, hatTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    hatModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's left hand, including laying down, lean forward, and shooting transitions.
 */
void Caine::drawLeftHand() const
{
    if (!leftHandLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftHandTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    Vec3 center = leftHandModel.getCenter();
    center.x -= 1.25f; // Offset to line up shoulder pivot
    float scale = 3.0f;

    // --- Pivot Matrix Sandwich for Left Hand ---
    // 1. Translate to shoulder pivot center in scaled space
    glTranslatef(center.x * scale, center.y * scale, center.z * scale);
    
    // 2. Apply shooting/pointing rotation sequence
    animation.applyShootingAnimation();

    // 3. Apply raising arm rotations when laying down
    glRotatef(animation.layDownFactor * 45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(animation.layDownFactor * 90.0f, 1.0f, 0.0f, 0.0f);

    // 4. Apply forward arm pitch when leaning forward
    glRotatef(animation.leanForwardFactor * 45.0f, 0.0f, 1.0f, 0.0f);

    // 5. Translate back from the shoulder pivot center
    glTranslatef(-center.x * scale, -center.y * scale, -center.z * scale);

    glScalef(scale, scale, scale);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftHandModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's left leg.
 */
void Caine::drawLeftLeg() const
{
    if (!leftLegLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftLegTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftLegModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's left palm, tracking parent hand position.
 */
void Caine::drawLeftPalm() const
{
    if (!leftPalmLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftPalmTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    Vec3 center = leftHandModel.getCenter();
    center.x -= 1.25f; // Match parental shoulder offset
    float scale = 3.0f;

    // --- Pivot Matrix Sandwich for Left Palm (inherits hand transformations) ---
    glTranslatef(center.x * scale, center.y * scale, center.z * scale);
    animation.applyShootingAnimation();
    glRotatef(animation.layDownFactor * 45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(animation.layDownFactor * 90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(animation.leanForwardFactor * 45.0f, 0.0f, 1.0f, 0.0f);
    glTranslatef(-center.x * scale, -center.y * scale, -center.z * scale);

    glScalef(scale, scale, scale);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftPalmModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's lower jaw, handling breathing mouth motions and death jaw drop.
 */
void Caine::drawLowerJaw() const
{
    if (!lowerJawLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, lowerJawTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    // Idle breathing TMJ swing
    glRotatef(animation.mouthOpenFactor * -5.0f, 1.0f, 0.0f, 0.0f);

    // Lean forward jaw rotation
    glRotatef(animation.leanForwardFactor * 10.0f, 0.0f, 1.0f, 0.0f);

    // Death wide mouth drop
    if (animation.isDead)
    {
        float deathFactor = animation.deathTimer;
        if (deathFactor > 1.0f) deathFactor = 1.0f;
        glRotatef(deathFactor * 15.0f, 0.0f, 1.0f, 0.0f);
    }

    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    lowerJawModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's right hand.
 */
void Caine::drawRightHand() const
{
    if (!rightHandLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightHandTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    Vec3 center = rightHandModel.getCenter();
    center.x += 1.25f; // Symmetrical shoulder alignment offset
    float scale = 3.0f;

    // --- Pivot Matrix Sandwich for Right Hand ---
    glTranslatef(center.x * scale, center.y * scale, center.z * scale);
    
    // Resting roll angle (fades out when laying down)
    float rightRoll = 30.0f * (1.0f - animation.layDownFactor);
    glRotatef(rightRoll, 0.0f, 0.0f, 1.0f);

    // Arm raising when laying down
    glRotatef(animation.layDownFactor * -45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(animation.layDownFactor * 90.0f, 1.0f, 0.0f, 0.0f);

    // Arm swing forward when leaning forward
    glRotatef(animation.leanForwardFactor * -45.0f, 0.0f, 1.0f, 0.0f);

    glTranslatef(-center.x * scale, -center.y * scale, -center.z * scale);

    glScalef(scale, scale, scale);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightHandModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's right leg.
 */
void Caine::drawRightLeg() const
{
    if (!rightLegLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightLegTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);
    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightLegModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's right palm.
 */
void Caine::drawRightPalm() const
{
    if (!rightPalmLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightPalmTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    if (rightHandLoaded)
    {
        Vec3 center = rightHandModel.getCenter();
        center.x += 1.25f; // Match right shoulder offset
        float scale = 3.0f;

        // Pivot Matrix Sandwich for Right Palm (inherits hand transformations)
        glTranslatef(center.x * scale, center.y * scale, center.z * scale);
        
        float rightRoll = 30.0f * (1.0f - animation.layDownFactor);
        glRotatef(rightRoll, 0.0f, 0.0f, 1.0f);
        
        glRotatef(animation.layDownFactor * -45.0f, 0.0f, 0.0f, 1.0f);
        glRotatef(animation.layDownFactor * 90.0f, 1.0f, 0.0f, 0.0f);
        glRotatef(animation.leanForwardFactor * -45.0f, 0.0f, 1.0f, 0.0f);
        
        glTranslatef(-center.x * scale, -center.y * scale, -center.z * scale);
    }

    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightPalmModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's staff, shifting alignment to the left hand grip space.
 */
void Caine::drawStaff() const
{
    if (!staffLoaded) return;

    // Local alignment offsets for Left Hand grip
    float staffOffsetX = 30.0f;
    float staffOffsetY = 18.0f;
    float staffOffsetZ = 3.0f;

    float staffGripRotX = 0.0f;
    float staffGripRotY = 0.0f;
    float staffGripRotZ = 0.0f;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, staffTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    Vec3 centerLeft = leftHandModel.getCenter();
    centerLeft.x -= 1.25f;
    float scale = 3.0f;

    // Apply left hand shooting/pointing offsets
    glTranslatef(centerLeft.x * scale, centerLeft.y * scale, centerLeft.z * scale);
    animation.applyShootingAnimation();
    glRotatef(animation.layDownFactor * 45.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(animation.layDownFactor * 90.0f, 1.0f, 0.0f, 0.0f);
    glTranslatef(-centerLeft.x * scale, -centerLeft.y * scale, -centerLeft.z * scale);

    // Shift coordinate reference frame from right hand to left hand
    if (rightHandLoaded)
    {
        Vec3 centerRight = rightHandModel.getCenter();
        Vec3 shift;
        shift.x = centerLeft.x - centerRight.x;
        shift.y = centerLeft.y - centerRight.y;
        shift.z = centerLeft.z - centerRight.z;
        glTranslatef(shift.x * scale, shift.y * scale, shift.z * scale);
    }

    // Apply custom offsets, grip orientations, body breathing tilt, and scales
    glTranslatef(staffOffsetX, staffOffsetY, staffOffsetZ);
    glRotatef(staffGripRotX, 1.0f, 0.0f, 0.0f);
    glRotatef(staffGripRotY, 0.0f, 1.0f, 0.0f);
    glRotatef(staffGripRotZ, 0.0f, 0.0f, 1.0f);
    glRotatef(animation.bodyTiltAngle, 0.0f, 0.0f, 1.0f);
    glScalef(scale, scale, scale);

    glColor3f(1.0f, 1.0f, 1.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    staffModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's tongue, with mouth opening movement on death.
 */
void Caine::drawTongue() const
{
    if (!tongueLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tongueTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);
    
    // Slight tongue drop flat on death mouth opening (rotated around pitch axis)
    if (animation.isDead)
    {
        float deathFactor = animation.deathTimer;
        if (deathFactor > 1.0f) deathFactor = 1.0f;
        glRotatef(deathFactor * 15.0f, 0.0f, 1.0f, 0.0f);
    }

    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    tongueModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's torso (Turso).
 */
void Caine::drawTurso() const
{
    if (!tursoLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tursoTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);
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
 * Render Caine's upper jaw, handling breathing TMJ movement, lean mouth open, and death mouth open.
 */
void Caine::drawUpperJaw() const
{
    if (!upperJawLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, upperJawTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(0.0f, -19.5f, 20.0f);

    // Breathing mouth flapping counter-rotation
    glRotatef(animation.mouthOpenFactor * -3.0f, 1.0f, 0.0f, 0.0f);

    // Lean forward mouth open rotation
    glRotatef(animation.leanForwardFactor * 5.0f, 1.0f, 0.0f, 0.0f);

    // Death mouth open rotation (pitch axis)
    if (animation.isDead)
    {
        float deathFactor = animation.deathTimer;
        if (deathFactor > 1.0f) deathFactor = 1.0f;
        glRotatef(deathFactor * 15.0f, 0.0f, 1.0f, 0.0f);
    }

    glScalef(3.0f, 3.0f, 3.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    upperJawModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's right eye.
 */
void Caine::drawRightEye() const
{
    if (!rightEyeLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, rightEyeTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(-0.5f, -9.0f, 20.0f);
    glScalef(4.0f, 4.0f, 4.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    rightEyeModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

/**
 * Render Caine's left eye.
 */
void Caine::drawLeftEye() const
{
    if (!leftEyeLoaded) return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, leftEyeTextureID);

    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glTranslatef(-0.5f, -9.0f, 20.0f);
    glScalef(4.0f, 4.0f, 4.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_NORMALIZE);
    leftEyeModel.draw();
    glDisable(GL_NORMALIZE);
    glEnable(GL_CULL_FACE);

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

static void drawWireCube(float size)
{
    float h = size / 2.0f;
    glBegin(GL_LINE_LOOP);
        glVertex3f(-h,  h, -h);
        glVertex3f( h,  h, -h);
        glVertex3f( h,  h,  h);
        glVertex3f(-h,  h,  h);
    glEnd();

    glBegin(GL_LINE_LOOP);
        glVertex3f(-h, -h, -h);
        glVertex3f( h, -h, -h);
        glVertex3f( h, -h,  h);
        glVertex3f(-h, -h,  h);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f(-h,  h, -h);
        glVertex3f(-h, -h, -h);

        glVertex3f( h,  h, -h);
        glVertex3f( h, -h, -h);

        glVertex3f( h,  h,  h);
        glVertex3f( h, -h,  h);

        glVertex3f(-h,  h,  h);
        glVertex3f(-h, -h,  h);
    glEnd();
}

/**
 * Render the entire Caine character, nesting all components under world space and posture transforms.
 */
void Caine::draw() const
{
    if (caineDeathSeqState >= 3)
        return;

    // Disappear completely after 1 second of dying
    if (animation.isDead && animation.deathTimer >= 1.0f)
        return;

    bool shouldRender = true;
    // Hide Caine (real or clone) when visualScaleFactor is zero (e.g. during DS state 1)
    if (visualScaleFactor <= 0.001f)
    {
        shouldRender = false;
    }
    else if (animation.isHurt)
    {
        // Toggle visibility every 0.05 seconds (10 Hz blink rate)
        if (std::fmod(animation.hurtTimer, 0.1f) < 0.05f)
        {
            shouldRender = false;
        }
    }

    if (shouldRender)
    {
        glPushMatrix();
        
        // 1. Move to world space coordinates
        glTranslatef(posX, posY, posZ);

        // Apply facing yaw so Caine faces the player
        glRotatef(facingYaw, 0.0f, 1.0f, 0.0f);

        // 2. Apply breathing hover offset
        glTranslatef(0.0f, animation.hoverOffset, 0.0f);

        // 3. Shift Caine upwards when laying down to keep above ground, and shift right/up to stay centered in hitbox
        glTranslatef(animation.layDownFactor * CAINE_LAYDOWN_SHIFT_X, 
                     animation.layDownFactor * (20.0f + CAINE_LAYDOWN_SHIFT_Y), 
                     0.0f);

        // 4. Transform to Caine's posture pivot center, apply posture rotations, scale, then translate back
        glTranslatef(0.0f, -19.5f, -20.0f);
        glRotatef(animation.bodyTiltAngle, 0.0f, 0.0f, 1.0f);          // Idle breathing body tilt
        glRotatef(animation.layDownFactor * 70.0f, 0.0f, 0.0f, 1.0f);    // Lay down transition rotation
        glRotatef(animation.leanForwardFactor * 45.0f, 1.0f, 0.0f, 0.0f); // Lean forward transition rotation
        float currentScale = uniformScale * visualScaleFactor;
        glScalef(currentScale, currentScale, currentScale);
        glTranslatef(0.0f, 19.5f, 20.0f);

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

        // 5. Draw the lower body limbs and torso
        drawTurso();
        drawLeftHand();
        drawRightHand();
        drawLeftPalm();
        drawRightPalm();
        drawLeftLeg();
        drawRightLeg();
        drawStaff();

        // 6. Draw the head parts grouped under a death tilt transform
        glPushMatrix();
        if (animation.isDead)
        {
            float deathFactor = animation.deathTimer;
            if (deathFactor > 1.0f) deathFactor = 1.0f;
            // Symmetrically rotate the head around local neck pivot
            glTranslatef(0.0f, 3.0f, -20.0f);
            glRotatef(deathFactor * 20.0f, 0.0f, 0.0f, 1.0f); // Tilt head to left
            glTranslatef(0.0f, -3.0f, 20.0f);
        }
        drawHat();
        drawUpperJaw();
        drawLowerJaw();
        drawTongue();
        drawRightEye();
        drawLeftEye();
        glPopMatrix();

        if (animation.isHurt || animation.isDead)
        {
            glPopAttrib();
        }

        glPopMatrix();
    }

    // Draw active projectiles in world space
    for (int i = 0; i < MAX_CAINE_PROJECTILES; i++)
    {
        if (projectiles[i].active)
        {
            if (difficultyLevel == 2 && currentPhase == 3)
            {
                glPushMatrix();
                glTranslatef(projectiles[i].posX, projectiles[i].posY, projectiles[i].posZ);
                
                glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
                glDisable(GL_LIGHTING);
                glDisable(GL_TEXTURE_2D);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                
                glColor4f(1.0f, 0.3f, 0.0f, 0.85f);
                drawSolidSphereFallback(2.5f, 8, 8);
                
                glColor4f(1.0f, 0.6f, 0.0f, 0.35f);
                drawSolidSphereFallback(4.0f, 8, 8);
                
                glPopAttrib();
                glPopMatrix();
            }
            else
            {
                myvirtualworld.environment.drawThinGlitchLineEffect(
                    projectiles[i].posX, 
                    projectiles[i].posY, 
                    projectiles[i].posZ, 
                    35.0f, 
                    1.0f,
                    projectiles[i].dirX,
                    projectiles[i].dirY,
                    projectiles[i].dirZ
                );
            }
        }
    }

    // Draw active teleport particles in world space
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    for (int i = 0; i < MAX_TELEPORT_PARTICLES; i++)
    {
        if (teleportParticles[i].active)
        {
            glPushMatrix();
            glTranslatef(teleportParticles[i].posX, teleportParticles[i].posY, teleportParticles[i].posZ);
            
            // Spin particles for a dynamic look
            float spin = teleportParticles[i].lifeTime * 250.0f;
            glRotatef(spin, 1.0f, 1.0f, 0.0f);
            
            glColor4f(teleportParticles[i].r, teleportParticles[i].g, teleportParticles[i].b, teleportParticles[i].alpha);
            
            // Draw a solid cube
            float s = teleportParticles[i].size * 0.5f;
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

    // Draw active sweep wall
    if (sweepActive)
    {
        float wallLength = 600.0f;
        int numPanels = 15;
        float step = wallLength / numPanels;
        float panelHalfWidth = step * 0.5f;
        float panelHalfHeight = 6.0f;
        float yCenter = -18.7f + panelHalfHeight; // sits on ground
        
        float rotationAngle = (sweepDirection == 0 || sweepDirection == 1) ? 0.0f : 90.0f;
        
        for (int i = 0; i < numPanels; i++)
        {
            float offset = -wallLength * 0.5f + i * step + panelHalfWidth;
            float px = 0.0f, py = yCenter, pz = 0.0f;
            if (rotationAngle == 0.0f)
            {
                px = offset;
                pz = sweepCurrentPos;
            }
            else
            {
                px = sweepCurrentPos;
                pz = offset;
            }
            
            float flicker = 0.5f + 0.5f * fabs(sin(myvirtualworld.environment.getAnimationTime() * 8.0f + i));
            float uShift = sin(myvirtualworld.environment.getAnimationTime() * 4.0f + i) * 0.2f;
            
            myvirtualworld.environment.drawGlitchPanelEffect(
                px, py, pz, 
                panelHalfWidth, panelHalfHeight, 
                uShift, 
                flicker, 
                rotationAngle
            );
        }
    }

    // Draw secondary active sweep wall
    if (sweepActive2)
    {
        float wallLength = 600.0f;
        int numPanels = 15;
        float step = wallLength / numPanels;
        float panelHalfWidth = step * 0.5f;
        float panelHalfHeight = 6.0f;
        float yCenter = -18.7f + panelHalfHeight; // sits on ground
        
        float rotationAngle = (sweepDirection2 == 0 || sweepDirection2 == 1) ? 0.0f : 90.0f;
        
        for (int i = 0; i < numPanels; i++)
        {
            float offset = -wallLength * 0.5f + i * step + panelHalfWidth;
            float px = 0.0f, py = yCenter, pz = 0.0f;
            if (rotationAngle == 0.0f)
            {
                px = offset;
                pz = sweepCurrentPos2;
            }
            else
            {
                px = sweepCurrentPos2;
                pz = offset;
            }
            
            float flicker = 0.5f + 0.5f * fabs(sin(myvirtualworld.environment.getAnimationTime() * 8.0f + i + 5));
            float uShift = sin(myvirtualworld.environment.getAnimationTime() * 4.0f + i + 5) * 0.2f;
            
            myvirtualworld.environment.drawGlitchPanelEffect(
                px, py, pz, 
                panelHalfWidth, panelHalfHeight, 
                uShift, 
                flicker, 
                rotationAngle
            );
        }
    }

    // Draw third active sweep wall
    if (sweepActive3)
    {
        float wallLength = 600.0f;
        int numPanels = 15;
        float step = wallLength / numPanels;
        float panelHalfWidth = step * 0.5f;
        float panelHalfHeight = 6.0f;
        float yCenter = -18.7f + panelHalfHeight; // sits on ground
        
        float rotationAngle = (sweepDirection3 == 0 || sweepDirection3 == 1) ? 0.0f : 90.0f;
        
        for (int i = 0; i < numPanels; i++)
        {
            float offset = -wallLength * 0.5f + i * step + panelHalfWidth;
            float px = 0.0f, py = yCenter, pz = 0.0f;
            if (rotationAngle == 0.0f)
            {
                px = offset;
                pz = sweepCurrentPos3;
            }
            else
            {
                px = sweepCurrentPos3;
                pz = offset;
            }
            
            float flicker = 0.5f + 0.5f * fabs(sin(myvirtualworld.environment.getAnimationTime() * 8.0f + i + 10));
            float uShift = sin(myvirtualworld.environment.getAnimationTime() * 4.0f + i + 10) * 0.2f;
            
            myvirtualworld.environment.drawGlitchPanelEffect(
                px, py, pz, 
                panelHalfWidth, panelHalfHeight, 
                uShift, 
                flicker, 
                rotationAngle
            );
        }
    }

    // Draw fourth active sweep wall
    if (sweepActive4)
    {
        float wallLength = 600.0f;
        int numPanels = 15;
        float step = wallLength / numPanels;
        float panelHalfWidth = step * 0.5f;
        float panelHalfHeight = 6.0f;
        float yCenter = -18.7f + panelHalfHeight; // sits on ground
        
        float rotationAngle = (sweepDirection4 == 0 || sweepDirection4 == 1) ? 0.0f : 90.0f;
        
        for (int i = 0; i < numPanels; i++)
        {
            float offset = -wallLength * 0.5f + i * step + panelHalfWidth;
            float px = 0.0f, py = yCenter, pz = 0.0f;
            if (rotationAngle == 0.0f)
            {
                px = offset;
                pz = sweepCurrentPos4;
            }
            else
            {
                px = sweepCurrentPos4;
                pz = offset;
            }
            
            float flicker = 0.5f + 0.5f * fabs(sin(myvirtualworld.environment.getAnimationTime() * 8.0f + i + 15));
            float uShift = sin(myvirtualworld.environment.getAnimationTime() * 4.0f + i + 15) * 0.2f;
            
            myvirtualworld.environment.drawGlitchPanelEffect(
                px, py, pz, 
                panelHalfWidth, panelHalfHeight, 
                uShift, 
                flicker, 
                rotationAngle
            );
        }
    }

    // Draw wireframe hitbox if showHitboxes is enabled
    if (showHitboxes && !animation.isDead)
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0f);

        Vec3 center = getCaineWorldCenter();
        float boxSize = 24.0f * uniformScale;

        glPushMatrix();
        glTranslatef(center.x, center.y, center.z);
        glColor3f(0.0f, 1.0f, 0.0f); // Bright green hitbox outline
        drawWireCube(boxSize);
        glPopMatrix();

        glPopAttrib();
    }

    // Draw clones during Doctor Strange state 2 in both test arena and normal gameplay
    if (!isClone && doctorStrangeState == 2)
    {
        for (int i = 0; i < MAX_CLONES; i++)
        {
            if (clones[i])
            {
                clones[i]->draw();
            }
        }
    }
}
