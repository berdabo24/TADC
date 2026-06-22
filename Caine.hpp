#ifndef CAINE_HPP
#define CAINE_HPP

#include <GL/glut.h>
#include <string>
#include "ObjModel.hpp"
#include "CaineAnimation.hpp"

namespace ProjectCaine
{

struct CaineProjectile
{
    bool active;
    float posX, posY, posZ;
    float dirX, dirY, dirZ;
    float lifeTimer;
};

struct TeleportParticle
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
 * Caine
 * Represents the Caine boss character, managing its models, textures, position, and drawing routines.
 *
 * Handles loading of individual OBJ model parts, binding associated texture IDs, scaling,
 * updating animation timelines, and executing hierarchical rendering of limbs, body, and head.
 */
class Caine
{
private:
    // OBJ Model Parts
    ObjModel hatModel;
    ObjModel leftHandModel;
    ObjModel leftLegModel;
    ObjModel leftPalmModel;
    ObjModel lowerJawModel;
    ObjModel rightHandModel;
    ObjModel rightLegModel;
    ObjModel rightPalmModel;
    ObjModel staffModel;
    ObjModel tongueModel;
    ObjModel tursoModel;
    ObjModel upperJawModel;
    ObjModel leftEyeModel;
    ObjModel rightEyeModel;

    // Loading Flags
    bool hatLoaded;
    bool leftHandLoaded;
    bool leftLegLoaded;
    bool leftPalmLoaded;
    bool lowerJawLoaded;
    bool rightHandLoaded;
    bool rightLegLoaded;
    bool rightPalmLoaded;
    bool staffLoaded;
    bool tongueLoaded;
    bool tursoLoaded;
    bool upperJawLoaded;
    bool leftEyeLoaded;
    bool rightEyeLoaded;

public:
    /**
     * Constructor that initializes position, spawn-tracking flags, and loading states to default.
     */
    Caine(bool isClone = false);
    ~Caine();
    void copyModelDataFrom(const Caine& other);

    /**
     * Updates the character's animation timelines, timers, and state transitions.
     * deltaTime The elapsed frame time in seconds.
     */
    void update(float deltaTime);

    CaineAnimation animation; // Animation state machine instance for this character

    float uniformScale; // Uniform scale multiplier for the entire 3D model representation
    float posX;         // Current X coordinate in 3D world space
    float posY;         // Current Y coordinate in 3D world space
    float posZ;         // Current Z coordinate in 3D world space

    float spawnX;              // Spawn point X coordinate (captured on first frame)
    float spawnY;              // Spawn point Y coordinate (captured on first frame)
    float spawnZ;              // Spawn point Z coordinate (captured on first frame)
    bool spawnPositionSaved;   // Flag tracking whether the spawn point has been successfully cached

    /**
     * Triggers the death sequence for Caine.
     *
     * Resets active animation postures, snaps Caine to his initial spawn position, and sets the death timer.
     */
    void triggerDeath();

    /**
     * Triggers the hurt sequence for Caine, initiating the red flash effect.
     */
    void triggerHurt();

    // AI state variables
    float aiFlightTimer;         // time elapsed in current flight path
    float aiFlightDuration;      // duration to fly in current direction before picking new target
    float aiTargetX;             // flight target X
    float aiTargetZ;             // flight target Z
    float aiTeleportTimer;       // timer tracking time until next teleport
    float aiTeleportInterval;    // random interval between teleports
    bool isTeleporting;          // shrinking phase
    bool isAppearing;            // growing phase
    float teleportTransitionTimer; // timer for scale shrinking/growing
    float visualScaleFactor;     // dynamic scale factor multiplied during teleport
    float facingYaw;             // horizontal angle facing the player

    void resetAI();

    /**
     * Sets the uniform scale of the character model.
     * scale The scaling factor to apply uniformly.
     */
    void setScale(float scale);

    int currentHealth;
    int maxHealth;
    void takeDamage(int amount);
    Vec3 getCaineWorldCenter() const;

    // Texture Bindings
    GLuint hatTextureID;
    GLuint leftHandTextureID;
    GLuint leftLegTextureID;
    GLuint leftPalmTextureID;
    GLuint lowerJawTextureID;
    GLuint rightHandTextureID;
    GLuint rightLegTextureID;
    GLuint rightPalmTextureID;
    GLuint staffTextureID;
    GLuint tongueTextureID;
    GLuint tursoTextureID;
    GLuint upperJawTextureID;
    GLuint leftEyeTextureID;
    GLuint rightEyeTextureID;

    // OBJ File Loading Interface
    bool loadHat(const std::string& filePath);
    bool loadLeftHand(const std::string& filePath);
    bool loadLeftLeg(const std::string& filePath);
    bool loadLeftPalm(const std::string& filePath);
    bool loadLowerJaw(const std::string& filePath);
    bool loadRightHand(const std::string& filePath);
    bool loadRightLeg(const std::string& filePath);
    bool loadRightPalm(const std::string& filePath);
    bool loadStaff(const std::string& filePath);
    bool loadTongue(const std::string& filePath);
    bool loadTurso(const std::string& filePath);
    bool loadUpperJaw(const std::string& filePath);
    bool loadRightEye(const std::string& filePath);
    bool loadLeftEye(const std::string& filePath);

    // Nested Rendering Functions for Model Parts
    void drawHat() const;
    void drawLeftHand() const;
    void drawLeftLeg() const;
    void drawLeftPalm() const;
    void drawLowerJaw() const;
    void drawRightHand() const;
    void drawRightLeg() const;
    void drawRightPalm() const;
    void drawStaff() const;
    void drawTongue() const;
    void drawTurso() const;
    void drawUpperJaw() const;
    void drawRightEye() const;
    void drawLeftEye() const;

    /**
     * Renders the entire character model hierarchically.
     *
     * Applies positioning translation, idle hovering offset, laying down transition,
     * leaning forward transition, death head tilting, and delegates drawing to the body/limb sub-functions.
     **/
    void draw() const;


    // Projectile systems
    static const int MAX_CAINE_PROJECTILES = 10;
    CaineProjectile projectiles[MAX_CAINE_PROJECTILES];
    float shootCooldownTimer;
    float shootInterval;
    void spawnProjectile();

    // Teleportation particle systems
    static const int MAX_TELEPORT_PARTICLES = 40;
    TeleportParticle teleportParticles[MAX_TELEPORT_PARTICLES];
    void spawnTeleportPoof(float x, float y, float z);

    // Sweep attack variables
    bool sweepActive;
    int sweepDirection;     // 0=N->S, 1=S->N, 2=W->E, 3=E->W  (active sweep)
    int nextSweepDirection; // Pre-rolled direction for the upcoming sweep (used by warning UI)
    float sweepCurrentPos;
    float sweepSpeed;
    float sweepTimer;
    float sweepInterval;
    bool wasLayingDown;
    bool testArenaSweepMode;

    bool sweepActive2;
    int sweepDirection2;
    int nextSweepDirection2;
    float sweepCurrentPos2;

    bool sweepActive3;
    int sweepDirection3;
    int nextSweepDirection3;
    float sweepCurrentPos3;

    bool sweepActive4;
    int sweepDirection4;
    int nextSweepDirection4;
    float sweepCurrentPos4;

    // Phase transition variables
    int currentPhase;
    bool isTransitioning;
    float transitionTimer;
    float transitionDuration;

    // Doctor Strange clone move variables
    bool isClone;
    static const int MAX_CLONES = 4;
    Caine* clones[MAX_CLONES];
    int doctorStrangeState; // 0=normal, 1=disappeared, 2=emergence, 3=stunned
    float doctorStrangeTimer;
    float particleSpawnTimer;
};

} // namespace ProjectCaine

#endif // CAINE_HPP
