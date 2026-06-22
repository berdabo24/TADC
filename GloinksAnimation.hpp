#ifndef GLOINKS_ANIMATION_HPP
#define GLOINKS_ANIMATION_HPP

#include <vector>
#include "ObjModel.hpp" // For Vec3

namespace ProjectGloinks
{

/**
 * Gloink
 * Struct containing state variables for a single Gloink instance.
 */
struct Gloink
{
    float posX;         // X coordinate of the Gloink in world space
    float posY;         // Y coordinate of the Gloink in world space (calculated via bounce)
    float posZ;         // Z coordinate of the Gloink in world space
    float jumpTimer;    // Timer tracking the jumping/bouncing curve
    int shapeType;      // Numeric identifier representing the specific shape model

    bool isHurt;        // Flag indicating if the Gloink has recently taken damage
    float hurtTimer;    // Timer tracking the duration of the hurt flashing/scaling effect

    int health;         // Lives remaining for this Gloink
    bool isDead;        // Flag indicating if the Gloink is in a dead/collapsing state
    float deathTimer;   // Timer tracking the progress of the Minecraft-style death animation

    bool isKnockedBack;
    float knockbackTimer;
    float knockbackDirX;
    float knockbackDirZ;
    bool isPaused;
    float pauseTimer;

    float groundLevel;  // Dynamic ground level base height for bouncing
};

/**
 * GloinksAnimation
 * Manages the collection of active Gloinks, their positions, state timers,
 * and helper functions to apply visual transformations/materials in OpenGL.
 */
class GloinksAnimation
{
public:
    std::vector<Gloink> activeGloinks; // Collection of active Gloink instances in the scene

    /**
     * Constructor that clears the collection of active Gloinks.
     */
    GloinksAnimation();
    
    float spawnTimer;
    float spawnInterval;
    int maxGloinks;
    void spawnGloink();

    /**
     * Spawns the initial set of 6 Gloinks spaced out along the X axis.
     */
    void initGloinks();

    /**
     * Updates positions and timers for all active Gloinks.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateGloinks(float deltaTime);

    /**
     * Damages a specific Gloink by index, triggering a hurt state or death.
     * index The index of the Gloink to damage.
     */
    void hurtGloink(int index);

    /**
     * Applies rotation, scaling, and fall translation animations to the active OpenGL matrix.
     * center The bounding box visual center of the shape model.
     * isDead Whether the Gloink is dead.
     * deathTimer The current progression timer of the death animation.
     * isHurt Whether the Gloink is hurt.
     * hurtTimer The current progression timer of the hurt animation.
     */
    void applyAnimationTransforms(Vec3 center, bool isDead, float deathTimer, bool isHurt, float hurtTimer) const;

    /**
     * Applies coloring/material properties based on the Gloink's current health state.
     * isDead Whether the Gloink is dead.
     * isHurt Whether the Gloink is hurt.
     * hurtTimer The current progression timer of the hurt animation.
     */
    void applyColorAndMaterial(bool isDead, bool isHurt, float hurtTimer) const;

    /**
     * Applies rotation on the Z-axis for rolling/spinning motion during jumping.
     * jumpTimer The jump timer value of the Gloink.
     * isDead Whether the Gloink is dead.
     */
    void applyRotation(float jumpTimer, bool isDead) const;

    /**
     * Restores normal material and lighting attributes.
     * isDead Whether the Gloink was dead.
     */
    void cleanupColorAndMaterial(bool isDead) const;
};

} // namespace ProjectGloinks

#endif // GLOINKS_ANIMATION_HPP
