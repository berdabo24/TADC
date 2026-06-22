#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "ObjModel.hpp"
#include <string>
#include <GL/glut.h>

namespace ProjectEnvironment
{
    struct Meteor
    {
        float originalX, originalY, originalZ;
        float currentX, currentY, currentZ;
        float startX, startY, startZ;
        float targetX, targetY, targetZ;
        float scale;
        float floatOffset;
        float floatAmplitude;
        int state; // 0 = floating, 1 = falling, 2 = hidden/respawning
        float fallProgress;
        float fallDuration;
        float respawnTimer;
    };

    struct MeteorParticle
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

    class Environment
    {
    private:
        ObjModel skyBoxModel;
        ObjModel groundModel;

        ObjModel castleWallModel;
        ObjModel cubeModel;
        ObjModel cubeGroupedModel;
        ObjModel irregularCubeModel;
        ObjModel pillarModel;
        ObjModel roofModel;
        ObjModel sphereModel;

        // Environment Animations
        float animationTime;

        // Environment Object Loader Checker
        bool skyBoxLoaded;
        bool groundLoaded;
        bool castleWallLoaded;
        bool cubeLoaded;
        bool cubeGroupedLoaded;
        bool irregularCubeLoaded;
        bool pillarLoaded;
        bool roofLoaded;
        bool sphereLoaded;

        // Render State Trackers
        mutable bool isCubeDrawn;
        mutable bool isPillarDrawn;
        mutable bool isIrregularCubeDrawn;
        mutable bool isCastleWallDrawn;

        // Texture IDs
        GLuint skyBoxTexture;
        GLuint groundTexture;
        GLuint roofTexture;
        GLuint castleWallTexture;
        GLuint cube1Texture;
        GLuint cube2Texture;
        GLuint cubeGroupedTexture;
        GLuint circusObject1Texture;
        GLuint circusObject2Texture;
        GLuint pillarTexture;
        GLuint skyTexture;

        // Animation Texture IDs
        GLuint glitchTexture;

    public:
        Environment();

        // Environment Animation
        void tickTime();
        float getAnimationTime() const { return animationTime; }

        // Texture Loading
        bool loadTextures();

        // Meteor Attack Move Variables
        static const int NUM_METEORS = 5;
        Meteor meteors[NUM_METEORS];
        static const int MAX_METEOR_PARTICLES = 300;
        MeteorParticle meteorParticles[MAX_METEOR_PARTICLES];
        bool isMeteorModeActive;
        float meteorCooldownTimer;
        float nextMeteorInterval;

        void resetMeteors();
        void updateMeteors(float deltaTime);
        void spawnMeteorTrailParticle(float x, float y, float z, float sizeFactor);
        void spawnMeteorBurst(float x, float y, float z);
        void spawnMeteorPoof(float x, float y, float z);
        void updateMeteorParticles(float deltaTime);
        void drawMeteorParticles() const;

        // Object Construction
        bool loadSkyBox(const std::string& filePath);
        bool loadGround(const std::string& filePath);
        bool loadRoof(const std::string& filePath);
        bool loadCastleWall(const std::string& filePath);
        bool loadCube(const std::string& filePath);
        bool loadCubeGrouped(const std::string& filePath);
        bool loadIrregularCube(const std::string& filePath);
        bool loadPillar(const std::string& filePath);
        bool loadSphere(const std::string& filePath);

        void drawSkyBox() const;
        void drawGround() const;
        void drawRoof() const;
        void drawCastleWall() const;
        void drawCube() const;
        void drawCubeGrouped() const;
        void drawIrregularCube() const;
        void drawPillar() const;
        void drawSphere() const;

        // Glitch Animation
        void drawDigitalEffect() const;
        void drawGlitchPanelEffect(float x = 0.0f, float y = 0.0f, float z = 0.0f, float width = 22.0f, float height = 8.0f, float uShift = 0.0f, float alpha = 1.0f, float rotationAngle = 0.0f) const;
        void drawThinGlitchLineEffect(float x = 0.0f, float y = 0.0f, float z = 0.0f, float length = 35.0f, float alpha = 1.0f, float dirX = 1.0f, float dirY = 0.057f, float dirZ = 0.0f) const;

        // Bounding Box Collision
        bool checkWallCollision(float playerX, float playerZ, float radius, float& outNewX, float& outNewZ) const;
        bool checkObstacleCollision(float playerX, float playerZ, float playerY, float radius, float& outNewX, float& outNewZ, float& outGroundY, bool isGloink = false) const;
        void getSkyBoxBounds(Vec3& minB, Vec3& maxB) const;

        //Main Draw Function
        void draw() const;
    };
}

#endif
