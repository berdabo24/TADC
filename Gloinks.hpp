#ifndef GLOINKS_HPP
#define GLOINKS_HPP

#include "ObjModel.hpp"
#include "GloinksAnimation.hpp"
#include <string>
#include <vector>

namespace ProjectGloinks
{

/**
 * Gloinks
 * Represents the Gloinks group actor, managing the underlying OBJ models, textures,
 * scaling, and forwarding calls to the state-driven GloinksAnimation module.
 */
class Gloinks
{
private:
    // OBJ Model Parts
    ObjModel gloinksBowlingPinModel;
    ObjModel gloinksCircleModel;
    ObjModel gloinksCubeModel;
    ObjModel gloinksMoonModel;
    ObjModel gloinksStarModel;
    ObjModel gloinksTriangularModel;

    // Loading Flags
    bool gloinksBowlingPinLoaded;
    bool gloinksCircleLoaded;
    bool gloinksCubeLoaded;
    bool gloinksMoonLoaded;
    bool gloinksStarLoaded;
    bool gloinksTriangularLoaded;

public:
    /**
     * Constructor that resets model loading flags and sets the initial uniform scale.
     */
    Gloinks();

    GloinksAnimation animation; // The active animation state-machine container

    float uniformScale; // Uniform scale multiplier for all Gloink instances

    /**
     * Sets the uniform scale of the Gloink shapes.
     * scale The scaling factor to apply uniformly.
     */
    void setScale(float scale);

    // Texture Bindings
    GLuint BowlingPinTextureID;
    GLuint CircleTextureID;
    GLuint CubeTextureID;
    GLuint MoonTextureID;
    GLuint StarTextureID;
    GLuint TriangularTextureID;

    // OBJ File Loading Interface
    bool loadGloinksBowlingPin(const std::string& filePath);
    bool loadGloinksCircle(const std::string& filePath);
    bool loadGloinksCube(const std::string& filePath);
    bool loadGloinksMoon(const std::string& filePath);
    bool loadGloinksStar(const std::string& filePath);
    bool loadGloinksTriangular(const std::string& filePath);

    // Render functions for individual shapes
    void drawGloinksBowlingPin(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;
    void drawGloinksCirle(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;
    void drawGloinksCube(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;
    void drawGloinksMoon(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;
    void drawGloinksStar(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;
    void drawGloinksTriangular(float jumpTimer, float posX, float posY, float posZ, bool isHurt, float hurtTimer, bool isDead, float deathTimer) const;

    /**
     * Renders all active Gloinks in the scene.
     */
    void draw() const;

    /**
     * Initializes all spawn variables and parameters for the collection of Gloinks.
     */
    void initGloinks();

    /**
     * Updates positions, timers, and states for all active Gloinks.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateGloinks(float deltaTime);

    /**
     * Damages a Gloink at the specified index.
     * index The index of the Gloink to damage.
     */
    void hurtGloink(int index);

    /**
     * Calculates the world space center of the Gloink at the specified index.
     * index The index of the Gloink.
     * returns The 3D world space coordinate of the Gloink's center.
     */
    Vec3 getGloinkWorldCenter(int index) const;
};

} // namespace ProjectGloinks

#endif // GLOINKS_HPP
