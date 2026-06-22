#ifndef KINGER_ANIMATION_HPP
#define KINGER_ANIMATION_HPP

// Constants to avoid magic numbers in both animation and rendering code
static const float BULLET_TRAVEL_SPEED   = 500.0f;
static const float BULLET_SPAWN_Z        = -25.0f; // Pushed out to the exact tip of the gun barrel
static const float BULLET_MAX_DISTANCE   = -10000.0f;
static const float BULLET_BARREL_Y_BIAS  = -2.0f;
static const float HURT_DURATION         = 0.5f;
static const float DEATH_DURATION        = 1.0f;
static const float RESPAWN_DELAY         = 1.0f;

/**
 * KingerAnimation
 * Manages the timelines, timers, states, and interpolation factors for the Kinger character.
 *
 * Tracks idle breathing sways, gunshot attack states, projectile trajectory calculations,
 * rolling roll states, reloading timelines, butterfly healing states, hurt stun states, and death sequence.
 */
class KingerAnimation
{
public:
    // Idle animation variables
    float idleTimer;        // Continuous time accumulator for idle breathing/bobbing
    float hoverOffset;      // Current vertical bobbing offset computed via sine wave
    float clothRotation;    // Current rotation angle for the cloth sway animation
    float armRotation;      // Current rotation angle for idle arm swaying
    float armSwayWeight;    // Current weight blending factor for idle sways vs active gestures

    // Gun Shot casting variables
    float skillTimer;        // Time progress tracker for the active gun casting skill
    bool  isCastingSkill;    // Flag indicating if the gunshot recoil and trajectory is active
    float skillArmRotation;  // Rotation offset applied to the arm during gun casting (deprecated/idle blend)
    float skillBodyYOffset;  // Vertical torso dip during gun shooting recoil (if any)
    float skillBodyZOffset;  // Depth torso kickback during gun shooting recoil (if any)
    float armRecoilOffset;   // Positional recoil displacement translation along local Z axis

    // Projectile state variables
    bool  isBulletActive;   // Flag indicating if the bullet is currently flying
    bool  hasSpawnedBullet; // Flag indicating if a bullet has been spawned in the current casting action
    float shootYaw;         // Saved camera/player yaw at the exact moment of bullet firing
    float shootPitch;       // Saved camera/player pitch at the exact moment of bullet firing
    float bulletPosX;       // Current X coordinate of the bullet in world space
    float bulletPosY;       // Current Y coordinate of the bullet in world space
    float bulletPosZ;       // Current Z coordinate of the bullet in world space
    
    // Trajectory Data
    float bulletStartX;     // Starting X position of the bullet at barrel tip
    float bulletStartY;     // Starting Y position of the bullet at barrel tip
    float bulletStartZ;     // Starting Z position of the bullet at barrel tip
    float bulletDirX;       // Trajectory direction unit vector X component
    float bulletDirY;       // Trajectory direction unit vector Y component
    float bulletDirZ;       // Trajectory direction unit vector Z component
    
    float bulletDistance;   // Total distance traveled by the active bullet
    float bulletLifeTimer;   // Timer tracking how long the bullet has been active

    // Roll State Variables
    bool  isRolling;        // Flag indicating if the roll ability is active
    float rollTimer;        // Timer tracking roll progress
    int   rollPhase;        // Numerical phase of the roll animation (squash, spin, recover)
    float rollSquashY;      // Squash and stretch scale factor applied on vertical axis
    bool  showBallModel;    // Flag indicating if Kinger should render as a rolling checker ball

    // Ammo & Reload State Variables
    int currentAmmo;        // Current ammo count left in the gun
    static const int MAX_AMMO = 5; // Max ammo capacity
    
    bool isReloading;       // Flag indicating if the reload animation is active
    float reloadTimer;      // Timer tracking reload progress
    float leftArmReloadPitch;   // Left arm Pitch rotation applied during reload
    float leftArmReloadYaw;     // Left arm Yaw rotation applied during reload
    float leftArmReloadYOffset; // Left arm vertical offset translation during reload
    float rightArmReloadPitch;  // Right arm Pitch rotation applied during reload
    float rightArmReloadYaw;    // Right arm Yaw rotation applied during reload
    
    // Butterfly Heal Variables
    int butterflyCharges;   // Remaining healing charges
    bool isHealing;         // Flag indicating if the butterfly healing animation is active
    float healTimer;        // Timer tracking heal animation progress
    float leftArmHealPitch; // Left arm Pitch rotation applied during heal gesture

    // Hurt/Damage State Variables
    bool isHurt;            // Flag indicating if the character has taken damage and is flashing
    float hurtTimer;        // Timer tracking hurt duration

    // Death State Variables
    bool isDead;            // Flag indicating if the character is dead
    float deathTimer;       // Timer tracking progress of the death falling sequence

    bool shouldSpawnMuzzleFlash; // Trigger flag for muzzle flash particle spawning

    /**
     * Constructor that resets all states, timers, and factors to their defaults.
     */
    KingerAnimation();

    /**
     * Updates the idle animation timer, hover heights, and cloth sway angles.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateIdleState(float deltaTime);

    /**
     * Casts the gun skill, reducing ammo count, and resetting shooting timers/recoil.
     */
    void castGunSkill();

    /**
     * Triggers the rolling movement ability, resetting the phase and squash states.
     */
    void castRollSkill(bool isGrounded);

    /**
     * Triggers the gun reload animation timeline.
     */
    void castReload();

    /**
     * Triggers the butterfly healing skill, reducing butterfly charges and restoring health.
     * currentHealth Reference to the character's current health.
     * maxHealth The maximum health threshold.
     */
    void castHealSkill(int& currentHealth, int maxHealth);

    /**
     * Updates the roll timeline, phases, and squash factors.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateRollState(float deltaTime);

    /**
     * Updates the reload timeline, animating left/right arms dynamically.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateReloadState(float deltaTime);

    /**
     * Updates the heal timeline, raising the left arm and spawning a butterfly.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateHealState(float deltaTime);

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
    void updateSkillState(float deltaTime, float currentYaw, float currentPitch, float kingerX, float kingerY, float kingerZ, float modelScale = 1.0f);

    /**
     * Updates the damage stun flashing timer.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateHurtState(float deltaTime);

    /**
     * Activates the hurt state, immediately interrupting any ongoing cast/reloads.
     */
    void triggerHurt();

    /**
     * Updates the death collapse timer.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateDeathState(float deltaTime);

    /**
     * Activates the death state, immediately interrupting any active skills.
     */
    void triggerDeath();
};

#endif // KINGER_ANIMATION_HPP
