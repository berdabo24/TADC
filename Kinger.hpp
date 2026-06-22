#ifndef KINGER_HPP
#define KINGER_HPP

#include "ObjModel.hpp"
#include "KingerAnimation.hpp"
#include <string>

namespace ProjectKinger
{

struct MuzzleParticle
{
    bool active;
    float posX, posY, posZ;
    float velX, velY, velZ;
    float size;
    float r, g, b;
    float alpha;
    float lifeTime;
    float maxLife;
};

/**
 * Kinger
 * Represents the main player character Kinger, managing individual model parts,
 * textures, movements, health status, and delegating to KingerAnimation.
 */
class Kinger
{
private:
    // OBJ Model Parts
    ObjModel headModel;
    ObjModel headPieceModel;
    ObjModel leftEyeModel;
    ObjModel rightEyeNModel;
    ObjModel bodyModel;
    ObjModel clothModel;
    ObjModel leftHandModel;
    ObjModel rightHandwGunModel;
    ObjModel bucketModel;
    ObjModel bucketHandleModel;

    // Loading Flags
    bool headLoaded;
    bool headPieceLoaded;
    bool leftEyeLoaded;
    bool rightEyeNLoaded;
    bool bodyLoaded;
    bool clothLoaded;
    bool leftHandLoaded;
    bool rightHandwGunLoaded;
    bool bucketLoaded;
    bool bucketHandleLoaded;

public:
    /**
     * Constructor that resets model loading flags, dimensions, properties, and health parameters.
     */
    Kinger();

    KingerAnimation animation; // State machine container holding all active animations

    int currentHealth; // Current health pool value
    int maxHealth;     // Maximum allowable health capacity

    /**
     * Updates physics, camera aiming orientations, input movement, and animation states.
     * deltaTime The elapsed frame time in seconds.
     * cameraYaw Current camera orientation yaw angle.
     * cameraPitch Current camera orientation pitch angle.
     * keyStates Array tracking keyboard/button inputs.
     */
    void update(float deltaTime, float cameraYaw, float cameraPitch, const bool* keyStates);
    
    float velocityY;    // Vertical speed velocity tracking jumping and gravity
    float knockbackVelX; // Horizontal knockback X velocity
    float knockbackVelZ; // Horizontal knockback Z velocity
    bool isGrounded;    // Flag indicating if Kinger is currently touching the floor
    float jumpScaleY;   // Squash and stretch scaling factor applied during landing/jumping impact
    
    /**
     * Initiates the jump movement by applying vertical velocity.
     */
    void jump();

    /**
     * Reduces health pool by a given amount and triggers hurt/death sequences.
     * amount Amount of damage to apply.
     */
    void takeDamage(int amount);

    /**
     * Resets health, velocities, coordinates, and transitions to normal idle alive state.
     */
    void rebirth();

    float aimPitch; // Visual alignment pitch matching target camera direction

    float posX;     // Current X coordinate of the player in world space
    float posY;     // Current Y coordinate of the player in world space
    float currentGroundY; // Current ground level Y coordinate under the player
    float posZ;     // Current Z coordinate of the player in world space
    float facingYaw; // Horizontal rotation facing direction in world space

    static constexpr float MAX_LEAN_ANGLE = 15.0f; // Maximum lean rotation tilt limit
    float targetLeanPitch;  // Target pitch tilting when running forward/backward
    float currentLeanPitch; // Interpolated running pitch tilt angle
    float targetLeanRoll;   // Target roll tilting when strafing left/right
    float currentLeanRoll;  // Interpolated running roll tilt angle

    float uniformScale; // Uniform scale multiplier for the 3D models

    /**
     * Sets the uniform scale of Kinger.
     * scale The scaling factor to apply uniformly.
     */
    void setScale(float scale);

    /**
     * Shifts player position along the forward orientation vector.
     * speed Movement speed multiplier.
     * cameraYaw Reference direction angle.
     */
    void moveForward(float speed, float cameraYaw);

    /**
     * Shifts player position along the lateral side orientation vector.
     * speed Movement speed multiplier.
     * cameraYaw Reference direction angle.
     */
    void moveRight(float speed, float cameraYaw);

    // Texture Bindings
    GLuint headTextureID;
    GLuint headPieceTextureID;
    GLuint clothTextureID;
    GLuint rightHandwGunTextureID;
    GLuint leftEyeTextureID;
    GLuint rightEyeNTextureID;

    // OBJ File Loaders
    bool loadHead(const std::string& filePath);
    bool loadHeadPiece(const std::string& filePath);
    bool loadLeftEye(const std::string& filePath);
    bool loadRightEyeN(const std::string& filePath);
    bool loadBody(const std::string& filePath);
    bool loadCloth(const std::string& filePath);
    bool loadLeftHand(const std::string& filePath);
    bool loadRightHandwGun(const std::string& filePath);
    bool loadBucket(const std::string& filePath);
    bool loadBucketHandle(const std::string& filePath);
    //bool load(const std::string& filePath);

    // Individual component rendering methods
    void drawHead() const;
    void drawHeadPiece() const;
    void drawLeftEye() const;
    void drawRightEyeN() const;
    void drawBody() const;
    void drawCloth() const;
    void drawLeftHand() const;
    void drawRightHandwGun() const;
    void drawBucket() const;
    void drawBucketHandle() const;
    void drawBullet() const;

    // Muzzle flash particle systems
    static const int MAX_MUZZLE_PARTICLES = 40;
    MuzzleParticle muzzleParticles[MAX_MUZZLE_PARTICLES];
    void spawnMuzzleFlash(float x, float y, float z);
    void updateMuzzleParticles(float deltaTime);
    void drawMuzzleParticles() const;

    /**
     * Renders the entire hierarchical character structure, combining limbs, bodies, and items.
     */
    void draw() const;
};

} // namespace ProjectKinger

#endif // KINGER_HPP
