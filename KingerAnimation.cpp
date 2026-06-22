#include "KingerAnimation.hpp"
#include <cmath>
#include "CNAWorld.hpp"

extern ProjectWorld::MyVirtualWorld myvirtualworld;
extern bool isTestArena;

static bool checkSegmentAABB(float x0, float y0, float z0, float x1, float y1, float z1,
                             float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
{
    float tmin = 0.0f;
    float tmax = 1.0f;

    // X axis
    float dx = x1 - x0;
    if (std::abs(dx) < 0.000001f)
    {
        if (x0 < minX || x0 > maxX) return false;
    }
    else
    {
        float t1 = (minX - x0) / dx;
        float t2 = (maxX - x0) / dx;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }

    // Y axis
    float dy = y1 - y0;
    if (std::abs(dy) < 0.000001f)
    {
        if (y0 < minY || y0 > maxY) return false;
    }
    else
    {
        float t1 = (minY - y0) / dy;
        float t2 = (maxY - y0) / dy;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }

    // Z axis
    float dz = z1 - z0;
    if (std::abs(dz) < 0.000001f)
    {
        if (z0 < minZ || z0 > maxZ) return false;
    }
    else
    {
        float t1 = (minZ - z0) / dz;
        float t2 = (maxZ - z0) / dz;
        tmin = std::max(tmin, std::min(t1, t2));
        tmax = std::min(tmax, std::max(t1, t2));
    }

    return tmin <= tmax;
}

/**
 * Constructor that resets all states, timers, and factors to their defaults.
 */
KingerAnimation::KingerAnimation()
    : idleTimer(0.0f)
    , hoverOffset(0.0f)
    , clothRotation(0.0f)
    , armRotation(0.0f)
    , armSwayWeight(1.0f)
    , skillTimer(0.0f)
    , isCastingSkill(false)
    , skillArmRotation(0.0f)
    , skillBodyYOffset(0.0f)
    , skillBodyZOffset(0.0f)
    , armRecoilOffset(0.0f)
    , isBulletActive(false)
    , hasSpawnedBullet(false)
    , shootYaw(0.0f)
    , shootPitch(0.0f)
    , bulletPosX(20.0f)
    , bulletPosY(0.0f)
    , bulletPosZ(0.0f)
    , bulletStartX(0.0f)
    , bulletStartY(0.0f)
    , bulletStartZ(0.0f)
    , bulletDirX(0.0f)
    , bulletDirY(0.0f)
    , bulletDirZ(0.0f)
    , bulletDistance(0.0f)
    , bulletLifeTimer(0.0f)
    , isRolling(false)
    , rollTimer(0.0f)
    , rollPhase(0)
    , rollSquashY(1.0f)
    , showBallModel(false)
    , currentAmmo(MAX_AMMO)
    , isReloading(false)
    , reloadTimer(0.0f)
    , leftArmReloadPitch(0.0f)
    , leftArmReloadYaw(0.0f)
    , leftArmReloadYOffset(0.0f)
    , rightArmReloadPitch(0.0f)
    , rightArmReloadYaw(0.0f)
    , butterflyCharges(3)
    , isHealing(false)
    , healTimer(0.0f)
    , leftArmHealPitch(0.0f)
    , isHurt(false)
    , hurtTimer(0.0f)
    , isDead(false)
    , deathTimer(0.0f)
    , shouldSpawnMuzzleFlash(false)
{
}

/**
 * Casts the gun skill, reducing ammo count, and resetting shooting timers/recoil.
 */
void KingerAnimation::castGunSkill()
{
    if (isRolling || isReloading || isHealing || isHurt) return;

    if (currentAmmo <= 0)
    {
        castReload();
        return;
    }

    if (!isCastingSkill)
    {
        extern bool isTestArena;
        if (!isTestArena)
        {
            currentAmmo--;
        }

        isCastingSkill  = true;
        skillTimer      = 0.0f;
        skillArmRotation  = 0.0f;
        skillBodyYOffset  = 0.0f;
        skillBodyZOffset  = 0.0f;
        armRecoilOffset   = 0.0f;
        isBulletActive = false;
        hasSpawnedBullet = false;
        shootYaw       = 0.0f;
        shootPitch     = 0.0f;
        bulletPosX     = 0.0f;
        bulletPosY     = 0.0f;
        bulletPosZ     = 0.0f;
        bulletDistance = 0.0f;
        
        if (currentAmmo <= 0)
        {
            castReload();
        }
    }
}

/**
 * Updates the gunshot recoil timeline and calculates active bullet positions.
 * deltaTime The elapsed frame time in seconds.
 * currentYaw Current facing horizontal yaw angle.
 * currentPitch Current aiming vertical pitch angle.
 * kingerX Current world X position of the player.
 * kingerY Current world Y position of the player.
 * kingerZ Current world Z position of the player.
 * modelScale Current uniform scale of the player model.
 */
void KingerAnimation::updateSkillState(float deltaTime, float currentYaw, float currentPitch, float kingerX, float kingerY, float kingerZ, float modelScale)
{
    if (isCastingSkill)
    {
        skillTimer += deltaTime;

        if (!hasSpawnedBullet)
        {
            hasSpawnedBullet = true;
            isBulletActive = true;
            shouldSpawnMuzzleFlash = true;
            shootYaw       = currentYaw;
            shootPitch     = currentPitch;
            bulletDistance = 0.0f;
            bulletLifeTimer = 0.0f;

            // Access the global camera variables updated in CNAmain.cpp
            extern float currentCameraEyeX;
            extern float currentCameraEyeY;
            extern float currentCameraEyeZ;

            extern float currentCameraDirX;
            extern float currentCameraDirY;
            extern float currentCameraDirZ;

            // 1. Set bullet start position directly to the center of the camera, offset by 10.0f units forward
            bulletStartX = currentCameraEyeX + currentCameraDirX * 40.0f;
            bulletStartY = (currentCameraEyeY - 5.0f) + currentCameraDirY * 40.0f;
            bulletStartZ = currentCameraEyeZ + currentCameraDirZ * 40.0f;

            bulletPosX = bulletStartX;
            bulletPosY = bulletStartY;
            bulletPosZ = bulletStartZ;

            // 2. Set trajectory direction directly to the camera's look direction
            bulletDirX = currentCameraDirX;
            bulletDirY = currentCameraDirY;
            bulletDirZ = currentCameraDirZ;
        }

        if (skillTimer <= 0.05f)
        {
            float t = skillTimer / 0.05f;
            armRecoilOffset = t * 3.0f;
        }
        else if (skillTimer <= 0.5f)
        {
            float t = (skillTimer - 0.05f) / 0.45f;
            float ease = 1.0f - std::cos(t * 3.14159265f / 2.0f); 
            armRecoilOffset = 3.0f * (1.0f - ease);
        }
        else
        {
            isCastingSkill   = false;
            skillTimer       = 0.0f;
            armRecoilOffset  = 0.0f;
            skillArmRotation = 0.0f;
            skillBodyYOffset = 0.0f;
            skillBodyZOffset = 0.0f;
        }
    }
    else
    {
        skillArmRotation = 0.0f;
        skillBodyYOffset = 0.0f;
        skillBodyZOffset = 0.0f;
    }

    if (isBulletActive)
    {
        bulletLifeTimer += deltaTime;
        if (bulletLifeTimer >= 2.0f)
        {
            isBulletActive = false;
        }
        else
        {
            float oldBulletPosX = bulletPosX;
            float oldBulletPosY = bulletPosY;
            float oldBulletPosZ = bulletPosZ;

            float step = BULLET_TRAVEL_SPEED * deltaTime;
            bulletDistance += step;

            bulletPosX += step * bulletDirX;
            bulletPosY += step * bulletDirY;
            bulletPosZ += step * bulletDirZ;

            // Check collision against Caine if he is active and alive
            if (::myvirtualworld.isCaineActive && !::myvirtualworld.caine.animation.isDead)
            {
                bool hitSomething = false;
                // If in Doctor Strange phase 2, check clones first!
                if (::myvirtualworld.caine.doctorStrangeState == 2)
                {
                    for (int i = 0; i < ProjectCaine::Caine::MAX_CLONES; i++)
                    {
                        ProjectCaine::Caine* clone = ::myvirtualworld.caine.clones[i];
                        if (clone && !clone->animation.isDead)
                        {
                            Vec3 cloneCenter = clone->getCaineWorldCenter();
                            float halfExtent = 12.0f * clone->uniformScale;

                            float minX = cloneCenter.x - halfExtent;
                            float maxX = cloneCenter.x + halfExtent;
                            float minY = cloneCenter.y - halfExtent;
                            float maxY = cloneCenter.y + halfExtent;
                            float minZ = cloneCenter.z - halfExtent;
                            float maxZ = cloneCenter.z + halfExtent;

                            if (checkSegmentAABB(oldBulletPosX, oldBulletPosY, oldBulletPosZ,
                                                 bulletPosX, bulletPosY, bulletPosZ,
                                                 minX, maxX, minY, maxY, minZ, maxZ))
                            {
                                clone->takeDamage(2);
                                isBulletActive = false;
                                hitSomething = true;
                                break;
                            }
                        }
                    }
                }

                if (!hitSomething)
                {
                    Vec3 caineCenter = ::myvirtualworld.caine.getCaineWorldCenter();
                    float halfExtent = 12.0f * ::myvirtualworld.caine.uniformScale;

                    float minX = caineCenter.x - halfExtent;
                    float maxX = caineCenter.x + halfExtent;
                    float minY = caineCenter.y - halfExtent;
                    float maxY = caineCenter.y + halfExtent;
                    float minZ = caineCenter.z - halfExtent;
                    float maxZ = caineCenter.z + halfExtent;

                    if (checkSegmentAABB(oldBulletPosX, oldBulletPosY, oldBulletPosZ,
                                         bulletPosX, bulletPosY, bulletPosZ,
                                         minX, maxX, minY, maxY, minZ, maxZ))
                    {
                        ::myvirtualworld.caine.takeDamage(5);
                        isBulletActive = false;
                    }
                }
            }

            // AABB Box Hitbox segment intersection check against all active Gloinks
            for (size_t i = 0; i < ::myvirtualworld.gloinks.animation.activeGloinks.size(); ++i)
            {
                auto& gloink = ::myvirtualworld.gloinks.animation.activeGloinks[i];
                if (gloink.isDead) continue;

                Vec3 gloinkCenter = ::myvirtualworld.gloinks.getGloinkWorldCenter(i);
                float halfExtent = 6.0f * ::myvirtualworld.gloinks.uniformScale;

                float minX = gloinkCenter.x - halfExtent;
                float maxX = gloinkCenter.x + halfExtent;
                float minY = gloinkCenter.y - halfExtent;
                float maxY = gloinkCenter.y + halfExtent;
                float minZ = gloinkCenter.z - halfExtent;
                float maxZ = gloinkCenter.z + halfExtent;

                if (checkSegmentAABB(oldBulletPosX, oldBulletPosY, oldBulletPosZ,
                                     bulletPosX, bulletPosY, bulletPosZ,
                                     minX, maxX, minY, maxY, minZ, maxZ))
                {
                    ::myvirtualworld.gloinks.hurtGloink(i);
                    isBulletActive = false;
                    break;
                }
            }

            if (isBulletActive && bulletDistance > -BULLET_MAX_DISTANCE) 
            {
                isBulletActive = false;
            }
        }
    }
}

/**
 * Updates the idle animation timer, hover heights, and cloth sway angles.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateIdleState(float deltaTime)
{
    idleTimer += deltaTime;

    hoverOffset = 1.2f * std::sin(idleTimer * 2.0f);

    clothRotation = 3.0f * std::cos(idleTimer * 2.5f);

    if (isCastingSkill)
    {
        armSwayWeight -= deltaTime * 15.0f; 
        if (armSwayWeight < 0.0f) armSwayWeight = 0.0f;
    }
    else
    {
        armSwayWeight += deltaTime * 5.0f; 
        if (armSwayWeight > 1.0f) armSwayWeight = 1.0f;
    }

    armRotation = (4.0f * std::sin(idleTimer * 1.8f)) * armSwayWeight;
}

/**
 * Triggers the rolling movement ability, resetting the phase and squash states.
 */
void KingerAnimation::castRollSkill(bool isGrounded)
{
    if (!isGrounded || isRolling || isReloading || isHealing) return;

    if (!isRolling)
    {
        isRolling = true;
        rollTimer = 0.0f;
        rollPhase = 0;
        rollSquashY = 1.0f;
        showBallModel = false;
    }
}

/**
 * Updates the roll timeline, phases, and squash factors.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateRollState(float deltaTime)
{
    if (!isRolling) return;

    rollTimer += deltaTime;

    if (rollTimer <= 0.1f)
    {
        rollPhase = 0;
        showBallModel = false;
        rollSquashY -= (1.0f - 0.3f) * (deltaTime / 0.1f);
        if (rollSquashY < 0.3f) rollSquashY = 0.3f;
    }
    else if (rollTimer <= 0.4f)
    {
        rollPhase = 1;
        showBallModel = true;
        rollSquashY = 1.0f;   
    }
    else if (rollTimer <= 0.5f)
    {
        if (rollPhase == 1) 
        {
            showBallModel = false; 
            rollSquashY = 0.3f;
        }
        rollPhase = 2;
        rollSquashY += (1.0f - 0.3f) * (deltaTime / 0.1f);
        if (rollSquashY > 1.0f) rollSquashY = 1.0f;
    }
    else
    {
        rollPhase = 3;
        isRolling = false;
        showBallModel = false;
        rollSquashY = 1.0f;
    }
}

/**
 * Triggers the gun reload animation timeline.
 */
void KingerAnimation::castReload()
{
    if (isReloading || isRolling || isHealing || isHurt) return;

    isReloading = true;
    reloadTimer = 0.0f;
    leftArmReloadPitch = 0.0f;
    leftArmReloadYaw = 0.0f;
    leftArmReloadYOffset = 0.0f;
    rightArmReloadPitch = 0.0f;
    rightArmReloadYaw = 0.0f;
}

/**
 * Updates the reload timeline, animating left/right arms dynamically.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateReloadState(float deltaTime)
{
    if (!isReloading) return;

    reloadTimer += deltaTime;

    if (reloadTimer <= 0.3f)
    {
        leftArmReloadPitch -= (120.0f) * (deltaTime / 0.3f);
        leftArmReloadYaw -= (135.0f) * (deltaTime / 0.3f);
        leftArmReloadYOffset += (10.0f) * (deltaTime / 0.3f); 
        rightArmReloadPitch -= (60.0f) * (deltaTime / 0.3f); 
        
        if (leftArmReloadPitch < -120.0f) leftArmReloadPitch = -120.0f;
        if (leftArmReloadYaw < -135.0f) leftArmReloadYaw = -135.0f;
        if (leftArmReloadYOffset > 10.0f) leftArmReloadYOffset = 10.0f;
        
        if (rightArmReloadPitch < -60.0f) rightArmReloadPitch = -60.0f;
    }
    else if (reloadTimer <= 0.7f)
    {
        leftArmReloadPitch = -120.0f + 5.0f * std::sin(reloadTimer * 30.0f);
        leftArmReloadYOffset = 10.0f; 
        rightArmReloadPitch = -60.0f + 2.0f * std::cos(reloadTimer * 30.0f); 
    }
    else if (reloadTimer <= 1.0f)
    {
        leftArmReloadPitch += (120.0f) * (deltaTime / 0.3f);
        leftArmReloadYaw += (135.0f) * (deltaTime / 0.3f);
        leftArmReloadYOffset -= (10.0f) * (deltaTime / 0.3f);
        rightArmReloadPitch += (60.0f) * (deltaTime / 0.3f);
        
        if (leftArmReloadPitch > 0.0f) leftArmReloadPitch = 0.0f;
        if (leftArmReloadYaw > 0.0f) leftArmReloadYaw = 0.0f;
        if (leftArmReloadYOffset < 0.0f) leftArmReloadYOffset = 0.0f;
        if (rightArmReloadPitch > 0.0f) rightArmReloadPitch = 0.0f;
    }
    else
    {
        isReloading = false;
        currentAmmo = MAX_AMMO;
        leftArmReloadPitch = 0.0f;
        leftArmReloadYaw = 0.0f;
        leftArmReloadYOffset = 0.0f;
        rightArmReloadPitch = 0.0f;
        rightArmReloadYaw = 0.0f;
    }
}

/**
 * Triggers the butterfly healing skill, reducing butterfly charges and restoring health.
 * currentHealth Reference to the character's current health.
 * maxHealth The maximum health threshold.
 */
void KingerAnimation::castHealSkill(int& currentHealth, int maxHealth)
{
    if (isHealing || isRolling || butterflyCharges <= 0 || isHurt || currentHealth >= maxHealth) return;

    isHealing = true;
    healTimer = 0.0f;
    leftArmHealPitch = 0.0f;

    extern bool isTestArena;
    if (!isTestArena)
    {
        butterflyCharges--;
    }

    currentHealth = maxHealth;
}

/**
 * Updates the heal timeline, raising the left arm and spawning a butterfly.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateHealState(float deltaTime)
{
    if (!isHealing) return;

    healTimer += deltaTime;

    if (healTimer <= 0.5f)
    {
        leftArmHealPitch -= (180.0f) * (deltaTime / 0.5f);
        if (leftArmHealPitch < -180.0f) leftArmHealPitch = -180.0f;
    }
    else if (healTimer <= 1.5f)
    {
        leftArmHealPitch = -180.0f;
    }
    else if (healTimer <= 2.0f)
    {
        leftArmHealPitch += (180.0f) * (deltaTime / 0.5f);
        if (leftArmHealPitch > 0.0f) leftArmHealPitch = 0.0f;
    }
    else
    {
        isHealing = false;
        leftArmHealPitch = 0.0f;
    }
}

/**
 * Updates the damage stun flashing timer.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateHurtState(float deltaTime)
{
    if (!isHurt) return;

    hurtTimer += deltaTime;
    if (hurtTimer >= HURT_DURATION)
    {
        isHurt = false;
        hurtTimer = 0.0f;
    }
}

/**
 * Activates the hurt state, immediately interrupting any ongoing cast/reloads.
 */
void KingerAnimation::triggerHurt()
{
    isHurt = true;
    hurtTimer = 0.0f;

    // Immediately stop ongoing skills
    isCastingSkill = false;
    skillTimer = 0.0f;
    skillArmRotation = 0.0f;
    skillBodyYOffset = 0.0f;
    skillBodyZOffset = 0.0f;
    armRecoilOffset = 0.0f;
    isBulletActive = false;
    hasSpawnedBullet = false;
    bulletDistance = 0.0f;

    isRolling = false;
    rollTimer = 0.0f;
    rollPhase = 0;
    rollSquashY = 1.0f;
    showBallModel = false;

    isReloading = false;
    reloadTimer = 0.0f;
    leftArmReloadPitch = 0.0f;
    leftArmReloadYaw = 0.0f;
    leftArmReloadYOffset = 0.0f;
    rightArmReloadPitch = 0.0f;
    rightArmReloadYaw = 0.0f;

    isHealing = false;
    healTimer = 0.0f;
    leftArmHealPitch = 0.0f;
}

/**
 * Updates the death collapse timer.
 * deltaTime The elapsed frame time in seconds.
 */
void KingerAnimation::updateDeathState(float deltaTime)
{
    if (!isDead) return;

    deathTimer += deltaTime;
    if (deathTimer > RESPAWN_DELAY)
    {
        deathTimer = RESPAWN_DELAY;
    }
}

/**
 * Activates the death state, immediately interrupting any active skills.
 */
void KingerAnimation::triggerDeath()
{
    isDead = true;
    deathTimer = 0.0f;

    // Stop background music when Kinger dies
    ::myvirtualworld.audioManager.stopBackgroundMusic();

    // Immediately stop ongoing skills
    isCastingSkill = false;
    skillTimer = 0.0f;
    skillArmRotation = 0.0f;
    skillBodyYOffset = 0.0f;
    skillBodyZOffset = 0.0f;
    armRecoilOffset = 0.0f;
    isBulletActive = false;
    hasSpawnedBullet = false;
    bulletDistance = 0.0f;

    isRolling = false;
    rollTimer = 0.0f;
    rollPhase = 0;
    rollSquashY = 1.0f;
    showBallModel = false;

    isReloading = false;
    reloadTimer = 0.0f;
    leftArmReloadPitch = 0.0f;
    leftArmReloadYaw = 0.0f;
    leftArmReloadYOffset = 0.0f;
    rightArmReloadPitch = 0.0f;
    rightArmReloadYaw = 0.0f;

    isHealing = false;
    healTimer = 0.0f;
    leftArmHealPitch = 0.0f;

    isHurt = false;
    hurtTimer = 0.0f;
}
