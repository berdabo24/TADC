/*
 TCG6223 Computer Graphics
 FIST, Multimedia University

 File: CNAmain.cpp
 Objective: Main Program for all file in the project

 Reference code from:
 Copyright (C) by Ya-Ping Wong <ypwong@mmu.edu.my>

 This file (CGLabmain.cpp) can be distributed to the students

 INSTRUCTIONS
 ============
 How to compile and run:
 * For each of the lab, you need the files below:

   a. CNAmain.cpp => The file you are reading now

   b. CNAmain.hpp => Header file to be used with CNAmain.cpp

   c. xx.cpp => Program for lab number 'xx', 'xx' being the lab
                     number such as 01, 03 ... 10, 11

   d. xx.hpp => Header file to be used with CGLabxx.cpp

 * Make sure you are including the correct include file
   in CNAmain.cpp (this file) such as:
      #include "xx.hpp"   where 'xx' is the function of the file

 * Make sure gl.h, glu.h and glut.h are in the 'include' path

 * Make sure opengl32.dll, glu32.dll and glut32.dll are in the 'system32' path

 * Make sure libopengl32.a, libglu32.a and libglut32.a are in the
   'lib' path and included in your project file if you are using an IDE

 * To compile using a command line using the GCC, type the command as below:
      g++ CNAmain.cpp CNAxx.cpp -o CNAxx.exe -lopengl32 -lglu32 -lglut32
      [replace 'xx' with the function of the file such as ObjModel, Kinger, GameLogic]

 How to modify:
 =============
 * All the user-defined drawing functions are called from
   the draw() function of class MyVirtualWorld which
   must be declared in CNAxx.hpp
 * myvirtualworld is declared in CNAmain.cpp as an instance
   of class MyVirtualWorld
 * MyVirtualWorld must implement the following member functions:
   a. draw()     => will be called by myDisplayFunc() of CNAmain.cpp
   b. tickTime() => will be called by myDisplayFunc() of CNAmain.cpp
                    for animation purposes
   c. init()     => will be called by myInit() of CNAmain.cpp
                    to carry up one-time setup/initialization before
                    any rendering BUT after OpenGL has been initialized
 * All classes and variables for each lab are defined in their own
   namespace to avoid names clashing of variable and type names.
   Specifically each CGLabxx.hpp declared and defined a MyVirtualWorld
   class, thus in CGLabmain.cpp, you need to specify from which
   namespace the MyVirtualWorld that you wish to use. Thus, you will
   need to modify the line (in the beginning of this program):
      CNAxx::MyVirtualWorld myvirtualworld;
   and change the 'xx' to the lab number that you wish to use.

   Game and 3d model by
   Group C&A


*/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <windows.h>
#include <GL/glut.h>

#include "CNAmain.hpp"

//Ideally, you should include only the files that you wish
//  to use, however, all of them are included here just for
//  convenience.
#include "CNAWorld.hpp"

//IMPORTANT:
//  Change the namespace scope below corresponding to
//  to the lab number which you wish to use.
using ProjectWorld::MyVirtualWorld;

MyVirtualWorld myvirtualworld;

using namespace std;

MyWindow   window;
MyWorld    world;
MyViewer   viewer;
MySetting  setting;
MyAxis     worldaxis;

static const float CAMERA_DISTANCE           = 60.0f;  // units BEHIND Kinger (must be positive)
static const float CAMERA_BASE_HEIGHT        = 30.0f;  // extra height above the target point
static const float CAMERA_SHOULDER_OFFSET    = 18.0f;  // Over-the-shoulder offset to the right
static const float CAMERA_TARGET_HEIGHT_OFFSET = 5.0f; // Kinger's visual centre above Y=0 (chest level)
static const float CAMERA_PITCH_MAX          =  1.2f;  // radians (~69 deg, camera high up)
static const float CAMERA_PITCH_MIN          = -1.2f;  // radians (steep upward aiming allowed)
static const float MOUSE_SENSITIVITY         =  0.003f;
static const float KINGER_MOVE_SPEED         =  1.0f;  // world units per key press
static const float CAMERA_KEY_TURN_INC       =  0.04f; // radians per arrow key

float boundaryScale = 14.5f; // Global scale for the logical boundary of Kinger and camera

float cameraYaw   = 0.0f;  // horizontal camera angle, radians
float cameraPitch = 0.4f;  // vertical camera angle, radians


float cameraTrackY = -18.7f;
const float CAMERA_FOLLOW_SPEED = 10.0f;

bool keyStates[256] = {false};

// Global variables tracking camera position and look direction
float currentCameraEyeX = 0.0f;
float currentCameraEyeY = 0.0f;
float currentCameraEyeZ = 0.0f;

float currentCameraDirX = 0.0f;
float currentCameraDirY = 0.0f;
float currentCameraDirZ = 0.0f;

bool showHitboxes = false; // Toggle to render green hitboxes around active Gloinks
enum GameUIState { START_MENU, GAMEPLAY, PAUSE_MENU, DEATH_SCREEN, WIN_SCREEN };
GameUIState currentUIState = START_MENU;
bool isWinDelayed = false;
float winDelayTimer = 0.0f;
bool isTestArena = false;
int difficultyLevel = 0; // 0 = Easy, 1 = Normal, 2 = Hard
float screenShakeTimer = 0.0f;
float screenShakeIntensity = 0.0f;

int caineDeathSeqState = 0;
float caineDeathSeqTimer = 0.0f;
int caineDeathSeqTextLength = 0;
float caineDeathSeqTextTimer = 0.0f;
std::string caineDeathSeqDialogue = "Uh.. wait-";  // Edit this string to change Caine's death dialogue
float deathSeqCamStartEyeX = 0.0f;
float deathSeqCamStartEyeY = 0.0f;
float deathSeqCamStartEyeZ = 0.0f;
float deathSeqCamStartTargetX = 0.0f;
float deathSeqCamStartTargetY = 0.0f;
float deathSeqCamStartTargetZ = 0.0f;
float currentCameraTargetX = 0.0f;
float currentCameraTargetY = 0.0f;
float currentCameraTargetZ = 0.0f;
bool hasStartedDeathSeq = false;

// Intro Sequence variables
int caineIntroSeqState = 0;
float caineIntroSeqTimer = 0.0f;
int caineIntroSeqTextLength = 0;
float caineIntroSeqTextTimer = 0.0f;
std::string caineIntroSeqDialogue = "Oh Kinger, there you are.";
float introSeqCamStartEyeX = 0.0f;
float introSeqCamStartEyeY = 0.0f;
float introSeqCamStartEyeZ = 0.0f;
float introSeqCamStartTargetX = 0.0f;
float introSeqCamStartTargetY = 0.0f;
float introSeqCamStartTargetZ = 0.0f;
bool hasStartedIntroSeq = false;

static void drawCenteredString(void* font, const char* str, float y, float left, float right);

void drawCrosshair()
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, window.width, 0, window.height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float cx = window.width / 2.0f;
    float cy = window.height / 2.0f;
    float size = 10.0f;

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
        glVertex2f(cx - size, cy);
        glVertex2f(cx + size, cy);
        glVertex2f(cx, cy - size);
        glVertex2f(cx, cy + size);
    glEnd();

    // Draw Phase Transition Text above crosshair
    if (myvirtualworld.isCaineActive && myvirtualworld.caine.isTransitioning)
    {
        float timer = myvirtualworld.caine.transitionTimer;
        if (timer <= 2.0f)
        {
            float alpha = 0.0f;
            if (timer <= 0.5f) {
                alpha = timer / 0.5f;
            } else if (timer <= 1.5f) {
                alpha = 1.0f;
            } else {
                alpha = 1.0f - (timer - 1.5f) / 0.5f;
            }

            // Phase 2 color: Gold/Orange. Phase 3 color: Crimson Red.
            if (myvirtualworld.caine.currentPhase == 2)
            {
                glColor4f(1.0f, 0.7f, 0.1f, alpha);
            }
            else if (myvirtualworld.caine.currentPhase == 3)
            {
                glColor4f(1.0f, 0.2f, 0.2f, alpha);
            }
            else
            {
                glColor4f(1.0f, 1.0f, 1.0f, alpha);
            }

            char phaseStr[32];
            sprintf(phaseStr, "PHASE %d", myvirtualworld.caine.currentPhase);
            drawCenteredString(GLUT_BITMAP_TIMES_ROMAN_24, phaseStr, cy + 30.0f, 0, window.width);
        }
    }

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

static void drawString(void* font, const char* str, float x, float y)
{
    glRasterPos2f(x, y);
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        str++;
    }
}

static void drawCenteredString(void* font, const char* str, float y, float left, float right);

void drawHUD()
{
    if (currentUIState == START_MENU)
        return;

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, window.width, 0, window.height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // ==========================================
    // 1. Kinger Health Bar (Bottom Left)
    // ==========================================
    // ==========================================
    // 1. Kinger Health Bar (Bottom Left)
    // ==========================================
    float barWidth = 250.0f;
    float barHeight = 20.0f;
    float left = 35.0f;
    float bottom = 35.0f;
    float right = left + barWidth;
    float top = bottom + barHeight;

    // Draw Health Bar & Ammo Labels
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    char kingerHealthText[64];
    sprintf(kingerHealthText, "KINGER  %d / %d", myvirtualworld.kinger.currentHealth, myvirtualworld.kinger.maxHealth);
    drawString(GLUT_BITMAP_HELVETICA_18, kingerHealthText, left, top + 8.0f);

    char kingerAmmoText[64];
    sprintf(kingerAmmoText, "Ammo: %d / %d", myvirtualworld.kinger.animation.currentAmmo, myvirtualworld.kinger.animation.MAX_AMMO);
    drawString(GLUT_BITMAP_HELVETICA_18, kingerAmmoText, left, top + 28.0f);

    char kingerHealText[64];
    sprintf(kingerHealText, "Heal: %d/%d", myvirtualworld.kinger.animation.butterflyCharges, difficultyLevel == 0 ? 5 : (difficultyLevel == 1 ? 3 : 1));
    drawString(GLUT_BITMAP_HELVETICA_18, kingerHealText, left, top + 48.0f);

    // Background Container
    glColor4f(0.08f, 0.08f, 0.12f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();

    // Health Fill
    float healthRatio = (float)myvirtualworld.kinger.currentHealth / myvirtualworld.kinger.maxHealth;
    if (healthRatio < 0.0f) healthRatio = 0.0f;
    if (healthRatio > 1.0f) healthRatio = 1.0f;
    float fillRight = left + barWidth * healthRatio;

    if (healthRatio > 0.0f)
    {
        glBegin(GL_QUADS);
            // Gradient fill (Circus Red-Orange)
            glColor4f(0.9f, 0.15f, 0.2f, 1.0f);
            glVertex2f(left, bottom);
            glColor4f(1.0f, 0.45f, 0.0f, 1.0f);
            glVertex2f(fillRight, bottom);
            glVertex2f(fillRight, top);
            glColor4f(0.9f, 0.15f, 0.2f, 1.0f);
            glVertex2f(left, top);
        glEnd();
    }

    // Border
    glLineWidth(1.5f);
    glColor4f(0.85f, 0.65f, 0.12f, 0.6f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();


    // ==========================================
    // 2. Control Tips (Bottom Right Side of Viewport)
    // ==========================================
    float tipsWidth = 250.0f;
    float tipsHeight = 110.0f;
    float tipsLeft = window.width - 35.0f - tipsWidth;
    float tipsBottom = 35.0f;
    float tipsRight = window.width - 35.0f;
    float tipsTop = tipsBottom + tipsHeight;

    // Background
    glColor4f(0.08f, 0.08f, 0.12f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(tipsLeft, tipsBottom);
        glVertex2f(tipsRight, tipsBottom);
        glVertex2f(tipsRight, tipsTop);
        glVertex2f(tipsLeft, tipsTop);
    glEnd();

    // Border
    glLineWidth(1.5f);
    glColor4f(0.85f, 0.65f, 0.12f, 0.5f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(tipsLeft, tipsBottom);
        glVertex2f(tipsRight, tipsBottom);
        glVertex2f(tipsRight, tipsTop);
        glVertex2f(tipsLeft, tipsTop);
    glEnd();

    float startY = tipsTop - 16.0f;
    float spacing = 16.0f;

    // W,A,S,D
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "WASD", tipsLeft + 15.0f, startY);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Move Forward/Sides", tipsLeft + 75.0f, startY);

    // Mouse1
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "Mouse1", tipsLeft + 15.0f, startY - spacing);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Shoot Weapon", tipsLeft + 75.0f, startY - spacing);

    // Space
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "Space", tipsLeft + 15.0f, startY - 2.0f * spacing);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Jump", tipsLeft + 75.0f, startY - 2.0f * spacing);

    // C
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "C", tipsLeft + 15.0f, startY - 3.0f * spacing);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Dodge Roll", tipsLeft + 75.0f, startY - 3.0f * spacing);

    // F
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "F", tipsLeft + 15.0f, startY - 4.0f * spacing);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Heal Skill", tipsLeft + 75.0f, startY - 4.0f * spacing);

    // R
    glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "R", tipsLeft + 15.0f, startY - 5.0f * spacing);
    glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_12, "- Reload Gun", tipsLeft + 75.0f, startY - 5.0f * spacing);

    if (isTestArena)
    {
        float taLeft = 30.0f;
        float taTop = window.height - 30.0f;
        float taWidth = 250.0f;
        float taHeight = 115.0f;
        float taRight = taLeft + taWidth;
        float taBottom = taTop - taHeight;

        // Background
        glColor4f(0.08f, 0.08f, 0.12f, 0.7f);
        glBegin(GL_QUADS);
            glVertex2f(taLeft, taBottom);
            glVertex2f(taRight, taBottom);
            glVertex2f(taRight, taTop);
            glVertex2f(taLeft, taTop);
        glEnd();

        // Border
        glLineWidth(1.5f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(taLeft, taBottom);
            glVertex2f(taRight, taBottom);
            glVertex2f(taRight, taTop);
            glVertex2f(taLeft, taTop);
        glEnd();

        float taStartY = taTop - 15.0f;
        float taSpacing = 18.0f;

        // Title
        glColor4f(1.0f, 0.84f, 0.0f, 1.0f); // Gold
        drawString(GLUT_BITMAP_HELVETICA_12, "TEST ARENA CONTROLS", taLeft + 12.0f, taStartY);

        taStartY -= 18.0f;
        // Key 1
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "[1]", taLeft + 12.0f, taStartY);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "Spawn Caine (Shoot & Move)", taLeft + 40.0f, taStartY);

        // Key 2
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "[2]", taLeft + 12.0f, taStartY - taSpacing);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "Spawn Caine (Sweep Attack)", taLeft + 40.0f, taStartY - taSpacing);

        // Key 3
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "[3]", taLeft + 12.0f, taStartY - 2.0f * taSpacing);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "Spawn Gloinks (Max 10)", taLeft + 40.0f, taStartY - 2.0f * taSpacing);

        // Key 4
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "[4]", taLeft + 12.0f, taStartY - 3.0f * taSpacing);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "Doctor Strange Clones Move", taLeft + 40.0f, taStartY - 3.0f * taSpacing);

        // Key 5
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "[5]", taLeft + 12.0f, taStartY - 4.0f * taSpacing);
        glColor4f(0.9f, 0.9f, 0.9f, 1.0f); drawString(GLUT_BITMAP_HELVETICA_10, "Meteor Attack Move", taLeft + 40.0f, taStartY - 4.0f * taSpacing);
    }

    // ==========================================
    // 3. Caine Boss Health Bar (Top Center)
    // ==========================================
    if (myvirtualworld.isCaineActive)
    {
        float cx = window.width / 2.0f;
        float bossWidth = 400.0f;
        float bossHeight = 18.0f;
        float bossLeft = cx - bossWidth / 2.0f;
        float bossRight = cx + bossWidth / 2.0f;
        float bossTop = window.height - 40.0f;
        float bossBottom = bossTop - bossHeight;

        // Label above
        glColor4f(1.0f, 0.85f, 0.2f, 1.0f); // Gold
        drawCenteredString(GLUT_BITMAP_TIMES_ROMAN_24, "CAINE", bossTop + 10.0f, bossLeft, bossRight);

        // Container Background
        glColor4f(0.08f, 0.08f, 0.12f, 0.8f);
        glBegin(GL_QUADS);
            glVertex2f(bossLeft, bossBottom);
            glVertex2f(bossRight, bossBottom);
            glVertex2f(bossRight, bossTop);
            glVertex2f(bossLeft, bossTop);
        glEnd();

        // Fill (Vibrant colors depending on Caine's current phase)
        float bossRatio = (float)myvirtualworld.caine.currentHealth / myvirtualworld.caine.maxHealth;
        if (bossRatio < 0.0f) bossRatio = 0.0f;
        if (bossRatio > 1.0f) bossRatio = 1.0f;
        float bossFillRight = bossLeft + bossWidth * bossRatio;

        if (bossRatio > 0.0f)
        {
            glBegin(GL_QUADS);
                // Color dynamically shifts depending on Caine's current phase
                float rStart = 0.7f, gStart = 0.1f, bStart = 0.7f; // Phase 1: Vibrant Purple
                float rEnd = 0.95f, gEnd = 0.2f, bEnd = 0.95f;

                if (myvirtualworld.caine.currentPhase == 2) // Phase 2: Orange/Gold
                {
                    rStart = 0.9f; gStart = 0.3f; bStart = 0.0f;
                    rEnd = 1.0f; gEnd = 0.6f; bEnd = 0.1f;
                }
                else if (myvirtualworld.caine.currentPhase == 3) // Phase 3: Crimson/Red
                {
                    rStart = 0.7f; gStart = 0.0f; bStart = 0.1f;
                    rEnd = 0.95f; gEnd = 0.1f; bEnd = 0.2f;
                }

                glColor4f(rStart, gStart, bStart, 1.0f);
                glVertex2f(bossLeft, bossBottom);
                glColor4f(rEnd, gEnd, bEnd, 1.0f);
                glVertex2f(bossFillRight, bossBottom);
                glVertex2f(bossFillRight, bossTop);
                glColor4f(rStart, gStart, bStart, 1.0f);
                glVertex2f(bossLeft, bossTop);
            glEnd();
        }

        // Border
        glLineWidth(2.0f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.8f); // Gold border
        glBegin(GL_LINE_LOOP);
            glVertex2f(bossLeft, bossBottom);
            glVertex2f(bossRight, bossBottom);
            glVertex2f(bossRight, bossTop);
            glVertex2f(bossLeft, bossTop);
        glEnd();

        // HP number label centered inside the bar
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        char bossHpText[32];
        sprintf(bossHpText, "HP: %d / %d", myvirtualworld.caine.currentHealth, myvirtualworld.caine.maxHealth);
        drawCenteredString(GLUT_BITMAP_HELVETICA_10, bossHpText, bossBottom + 4.0f, bossLeft, bossRight);
    }

    // ==========================================
    // 4. Sweep Attack Warning Indicator
    // ==========================================
    // Show a flashing directional triangle at the screen edge during the 2-second delay
    // before Caine's sweep wall becomes active. Direction: 0=N->S, 1=S->N, 2=W->E, 3=E->W
    if (myvirtualworld.isCaineActive &&
        myvirtualworld.caine.animation.isLayingDown &&
        !myvirtualworld.caine.sweepActive)
    {
        int numWarnings = 1;
        if (difficultyLevel == 2)
        {
            if (myvirtualworld.caine.currentPhase == 1) numWarnings = 2;
            else if (myvirtualworld.caine.currentPhase == 2) numWarnings = 3;
            else if (myvirtualworld.caine.currentPhase == 3) numWarnings = 4;
        }
        else
        {
            numWarnings = (myvirtualworld.caine.currentPhase == 3) ? 2 : 1;
        }

        for (int wIdx = 0; wIdx < numWarnings; wIdx++)
        {
            int sweepDir = 0;
            if (wIdx == 0) sweepDir = myvirtualworld.caine.nextSweepDirection;
            else if (wIdx == 1) sweepDir = myvirtualworld.caine.nextSweepDirection2;
            else if (wIdx == 2) sweepDir = myvirtualworld.caine.nextSweepDirection3;
            else if (wIdx == 3) sweepDir = myvirtualworld.caine.nextSweepDirection4;

            float sweepT = myvirtualworld.caine.sweepTimer;
            float sweepI = myvirtualworld.caine.sweepInterval;

            // Flash frequency increases as the attack approaches (1 Hz to 8 Hz)
            float progress = (sweepI > 0.0f) ? (sweepT / sweepI) : 0.0f;
            float flashFreq = 1.0f + progress * 7.0f;
            // Smooth sine-wave pulse alpha (fades in/out, intensifies near strike)
            float pulseAlpha = 0.25f + 0.5f * (0.5f + 0.5f * std::sin(sweepT * flashFreq * 2.0f * 3.14159265f));

            float W = (float)window.width;
            float H = (float)window.height;
            float cx2 = W * 0.5f;
            float cy2 = H * 0.5f;

            // Triangle size
            float triBase  = 60.0f; // half-width of the triangle base
            float triDepth = 40.0f; // how far the tip juts inward from the base
            float radius   = std::min(W, H) * 0.25f; // base distance from screen center

            // Compute threat direction unit vector (dx, dy) in screen coordinates
            float dx = 0.0f;
            float dy = 0.0f;
            if (sweepDir == 0) // North
            {
                dx = std::sin(cameraYaw);
                dy = std::cos(cameraYaw);
            }
            else if (sweepDir == 1) // South
            {
                dx = -std::sin(cameraYaw);
                dy = -std::cos(cameraYaw);
            }
            else if (sweepDir == 2) // West
            {
                dx = -std::cos(cameraYaw);
                dy = std::sin(cameraYaw);
            }
            else // East (sweepDir == 3)
            {
                dx = std::cos(cameraYaw);
                dy = -std::sin(cameraYaw);
            }

            // Normalize threat direction vector to avoid floating point issues
            float len = std::sqrt(dx * dx + dy * dy);
            if (len > 0.0f)
            {
                dx /= len;
                dy /= len;
            }

            // Base center of threat triangle
            float bx = cx2 + radius * dx;
            float by = cy2 + radius * dy;

            // Tip pointing inwards (towards center)
            float tx = cx2 + (radius - triDepth) * dx;
            float ty = cy2 + (radius - triDepth) * dy;

            // Base corners offset perpendicularly from base center (bx, by)
            float b1x = bx - triBase * dy;
            float b1y = by + triBase * dx;
            float b2x = bx + triBase * dy;
            float b2y = by - triBase * dx;

            // Warning label text position (aligned outside the base, further away from center)
            float labelDist = radius + 15.0f;
            float lx = cx2 + labelDist * dx;
            float ly = cy2 + labelDist * dy;
            float labelX = lx - 26.0f;
            float labelY = ly - 5.0f;
            const char* warningLabel = "! SWEEP !";

            // Draw filled triangle with additive-style blending for glow effect
            glColor4f(1.0f, 0.08f, 0.08f, pulseAlpha);
            glBegin(GL_TRIANGLES);
                glVertex2f(tx,  ty);
                glVertex2f(b1x, b1y);
                glVertex2f(b2x, b2y);
            glEnd();

            // Draw bright red outline for crispness
            glLineWidth(2.5f);
            glColor4f(1.0f, 0.3f, 0.3f, pulseAlpha + 0.2f > 1.0f ? 1.0f : pulseAlpha + 0.2f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(tx,  ty);
                glVertex2f(b1x, b1y);
                glVertex2f(b2x, b2y);
            glEnd();

            // Draw warning text label
            glColor4f(1.0f, 1.0f, 0.2f, pulseAlpha + 0.2f > 1.0f ? 1.0f : pulseAlpha + 0.2f);
            drawString(GLUT_BITMAP_HELVETICA_12, warningLabel, labelX, labelY);
        }
    }

    // RPG Dialogue Box (Caine Death/Intro sequence States 2 & 3)
    if (caineDeathSeqState == 2 || caineDeathSeqState == 3 || caineIntroSeqState == 2 || caineIntroSeqState == 3)
    {
        float boxW = 600.0f;
        float boxH = 120.0f;
        if (boxW > window.width * 0.9f) {
            boxW = window.width * 0.9f;
        }
        float bLeft = (window.width - boxW) / 2.0f;
        float bRight = bLeft + boxW;
        float bBottom = 40.0f;
        float bTop = bBottom + boxH;

        // Draw solid black background
        glColor4f(0.0f, 0.0f, 0.0f, 0.95f);
        glBegin(GL_QUADS);
            glVertex2f(bLeft, bBottom);
            glVertex2f(bRight, bBottom);
            glVertex2f(bRight, bTop);
            glVertex2f(bLeft, bTop);
        glEnd();

        // Draw thick white border
        glLineWidth(4.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(bLeft, bBottom);
            glVertex2f(bRight, bBottom);
            glVertex2f(bRight, bTop);
            glVertex2f(bLeft, bTop);
        glEnd();

        // Draw title "Caine" in gold
        glColor4f(1.0f, 0.84f, 0.0f, 1.0f); // Gold
        drawString(GLUT_BITMAP_HELVETICA_18, "Caine", bLeft + 30.0f, bTop - 30.0f);

        // Draw scrolled dialogue text in white
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        std::string fullText = (caineIntroSeqState > 0) ? caineIntroSeqDialogue : caineDeathSeqDialogue;
        int textLength = (caineIntroSeqState > 0) ? caineIntroSeqTextLength : caineDeathSeqTextLength;
        bool isFullState = (caineIntroSeqState > 0) ? (caineIntroSeqState == 3) : (caineDeathSeqState == 3);
        std::string subText = isFullState ? fullText : fullText.substr(0, textLength);
        drawString(GLUT_BITMAP_HELVETICA_18, subText.c_str(), bLeft + 30.0f, bTop - 75.0f);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

static void drawCenteredString(void* font, const char* str, float y, float left, float right)
{
    int len = strlen(str);
    float charWidth = 10.0f; // estimation for Helvetica 18
    if (font == GLUT_BITMAP_HELVETICA_12) charWidth = 7.0f;
    else if (font == GLUT_BITMAP_HELVETICA_18) charWidth = 11.0f;
    else if (font == GLUT_BITMAP_TIMES_ROMAN_24) charWidth = 14.0f;
    float textWidth = len * charWidth;
    float x = (left + right)/2.0f - textWidth/2.0f;
    glRasterPos2f(x, y);
    while (*str)
    {
        glutBitmapCharacter(font, *str);
        str++;
    }
}

void drawMenuUI()
{
    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, window.width, 0, window.height);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float cx = window.width / 2.0f;
    float cy = window.height / 2.0f;

    if (currentUIState == START_MENU)
    {
        // 1. Draw Semi-transparent Full-Screen Gray Background
        glColor4f(0.1f, 0.1f, 0.1f, 0.65f); // Transparent Dark Gray
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(window.width, 0.0f);
            glVertex2f(window.width, window.height);
            glVertex2f(0.0f, window.height);
        glEnd();

        float width = 450.0f;
        float height = 360.0f;
        float left = cx - width / 2.0f;
        float right = cx + width / 2.0f;
        float top = cy + height / 2.0f;
        float bottom = cy - height / 2.0f;

        // Draw semi-transparent background gradient
        glBegin(GL_QUADS);
            // Top-left: deep velvet red
            glColor4f(0.4f, 0.05f, 0.08f, 0.9f);
            glVertex2f(left, top);
            // Top-right: dark midnight blue
            glColor4f(0.05f, 0.08f, 0.15f, 0.9f);
            glVertex2f(right, top);
            // Bottom-right: dark midnight blue
            glColor4f(0.02f, 0.04f, 0.08f, 0.95f);
            glVertex2f(right, bottom);
            // Bottom-left: deep velvet red
            glColor4f(0.15f, 0.02f, 0.04f, 0.95f);
            glVertex2f(left, bottom);
        glEnd();

        // Sleek double gold border with pulse animation
        float tTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float pulse = 0.85f + 0.15f * std::sin(tTime * 3.5f);

        glLineWidth(3.0f);
        glColor4f(0.85f * pulse, 0.65f * pulse, 0.12f * pulse, 0.9f); // gold pulse
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        glLineWidth(1.0f);
        glColor4f(0.9f, 0.8f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left + 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, top - 5.0f);
            glVertex2f(left + 5.0f, top - 5.0f);
        glEnd();

        // Title text
        glColor4f(1.0f, 0.85f, 0.2f, 1.0f); // Bright Gold
        drawCenteredString(GLUT_BITMAP_TIMES_ROMAN_24, "THE AMAZING DIGITAL CIRCUS", top - 45.0f, left, right);
        glColor4f(0.9f, 0.9f, 0.9f, 0.8f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "3D COMPUTER GRAPHICS ADVENTURE", top - 65.0f, left, right);

        // Divider
        glLineWidth(2.0f);
        glBegin(GL_LINES);
            glColor4f(0.85f, 0.65f, 0.12f, 0.1f); glVertex2f(left + 40.0f, top - 80.0f);
            glColor4f(0.85f, 0.65f, 0.12f, 0.9f); glVertex2f(cx, top - 80.0f);
            glColor4f(0.85f, 0.65f, 0.12f, 0.9f); glVertex2f(cx, top - 80.0f);
            glColor4f(0.85f, 0.65f, 0.12f, 0.1f); glVertex2f(right - 40.0f, top - 80.0f);
        glEnd();

        // Options
        float startY = top - 120.0f;
        float spacing = 40.0f;

        // Start Game
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[1]", left + 60.0f, startY);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Start Game", left + 100.0f, startY);

        // Debug Environment
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[2]", left + 60.0f, startY - spacing);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Debug Environment", left + 100.0f, startY - spacing);

        // Test Arena
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[3]", left + 60.0f, startY - 2.0f * spacing);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Test Arena", left + 100.0f, startY - 2.0f * spacing);

        // Difficulty Setting
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[4]", left + 60.0f, startY - 3.0f * spacing);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        std::string diffText = "Difficulty: ";
        if (difficultyLevel == 0) diffText += "EASY";
        else if (difficultyLevel == 1) diffText += "NORMAL";
        else if (difficultyLevel == 2) diffText += "HARD";
        drawString(GLUT_BITMAP_HELVETICA_18, diffText.c_str(), left + 100.0f, startY - 3.0f * spacing);

        // Exit Game
        glColor4f(1.0f, 0.3f, 0.3f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[0]", left + 60.0f, startY - 4.0f * spacing - 15.0f);
        glColor4f(0.90f, 0.90f, 0.90f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Exit Game", left + 100.0f, startY - 4.0f * spacing - 15.0f);

        // Footer
        glColor4f(0.6f, 0.6f, 0.6f, 0.7f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "Press numeric keys to select an option", bottom + 25.0f, left, right);
    }
    else if (currentUIState == PAUSE_MENU)
    {
        // 1. Draw Semi-transparent Full-Screen Gray Background
        glColor4f(0.1f, 0.1f, 0.1f, 0.65f); // Transparent Dark Gray
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(window.width, 0.0f);
            glVertex2f(window.width, window.height);
            glVertex2f(0.0f, window.height);
        glEnd();

        float width = 400.0f;
        float height = 250.0f;
        float left = cx - width / 2.0f;
        float right = cx + width / 2.0f;
        float top = cy + height / 2.0f;
        float bottom = cy - height / 2.0f;

        // Background
        glBegin(GL_QUADS);
            glColor4f(0.05f, 0.08f, 0.15f, 0.9f); // Dark blue-gray top
            glVertex2f(left, top);
            glVertex2f(right, top);
            glColor4f(0.02f, 0.04f, 0.08f, 0.95f); // Darker bottom
            glVertex2f(right, bottom);
            glVertex2f(left, bottom);
        glEnd();

        // Border
        glLineWidth(2.0f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.8f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        // Title
        glColor4f(1.0f, 0.85f, 0.2f, 1.0f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_18, "GAME PAUSED", top - 35.0f, left, right);

        // Divider
        glLineWidth(1.0f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.4f);
        glBegin(GL_LINES);
            glVertex2f(left + 30.0f, top - 48.0f);
            glVertex2f(right - 30.0f, top - 48.0f);
        glEnd();

        // Options
        float startY = top - 85.0f;
        float spacing = 35.0f;

        // Option 1: Reset Game
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[1]", left + 40.0f, startY);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Restart", left + 80.0f, startY);

        // Option 2: Return to Menu
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[2]", left + 40.0f, startY - spacing);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Return to Main Menu", left + 80.0f, startY - spacing);

        // Option 3: Resume
        glColor4f(0.7f, 0.7f, 0.8f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[0]", left + 40.0f, startY - 2.0f * spacing - 10.0f);
        glColor4f(0.90f, 0.90f, 0.90f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Resume Game", left + 80.0f, startY - 2.0f * spacing - 10.0f);

        // Footer
        glColor4f(0.6f, 0.6f, 0.6f, 0.7f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "Press [ESC], [z], or [0] to Resume", bottom + 20.0f, left, right);
    }
    else if (currentUIState == DEATH_SCREEN)
    {
        // 1. Draw Semi-transparent Full-Screen Red Tint Background
        glColor4f(0.15f, 0.02f, 0.02f, 0.65f); // transparent dark red
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(window.width, 0.0f);
            glVertex2f(window.width, window.height);
            glVertex2f(0.0f, window.height);
        glEnd();

        float width = 400.0f;
        float height = 250.0f;
        float left = cx - width / 2.0f;
        float right = cx + width / 2.0f;
        float top = cy + height / 2.0f;
        float bottom = cy - height / 2.0f;

        // Background
        glBegin(GL_QUADS);
            glColor4f(0.25f, 0.02f, 0.04f, 0.9f); // Dark burgundy top
            glVertex2f(left, top);
            glVertex2f(right, top);
            glColor4f(0.08f, 0.01f, 0.02f, 0.95f); // Pitch black-red bottom
            glVertex2f(right, bottom);
            glVertex2f(left, bottom);
        glEnd();

        // Sleek double gold border with pulse animation
        float tTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float pulse = 0.85f + 0.15f * std::sin(tTime * 3.5f);

        glLineWidth(3.0f);
        glColor4f(0.85f * pulse, 0.65f * pulse, 0.12f * pulse, 0.9f); // gold pulse
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        glLineWidth(1.0f);
        glColor4f(0.9f, 0.8f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left + 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, top - 5.0f);
            glVertex2f(left + 5.0f, top - 5.0f);
        glEnd();

        // Title
        glColor4f(1.0f, 0.15f, 0.15f, 1.0f); // Ominous Red
        drawCenteredString(GLUT_BITMAP_TIMES_ROMAN_24, "YOU DIED", top - 45.0f, left, right);

        // Divider
        glLineWidth(1.0f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.4f);
        glBegin(GL_LINES);
            glVertex2f(left + 30.0f, top - 65.0f);
            glVertex2f(right - 30.0f, top - 65.0f);
        glEnd();

        // Options
        float startY = top - 110.0f;
        float spacing = 45.0f;

        // Restart
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[1]", left + 50.0f, startY);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Restart Game", left + 90.0f, startY);

        // Return to Menu
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[2]", left + 50.0f, startY - spacing);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Return to Main Menu", left + 90.0f, startY - spacing);

        // Footer
        glColor4f(0.6f, 0.6f, 0.6f, 0.7f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "Press numeric keys to select an option", bottom + 25.0f, left, right);
    }
    else if (currentUIState == WIN_SCREEN)
    {
        // 1. Draw Semi-transparent Full-Screen Blue/Teal Tint Background
        glColor4f(0.02f, 0.1f, 0.15f, 0.65f); // transparent dark teal/blue
        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0f);
            glVertex2f(window.width, 0.0f);
            glVertex2f(window.width, window.height);
            glVertex2f(0.0f, window.height);
        glEnd();

        float width = 450.0f;
        float height = 280.0f;
        float left = cx - width / 2.0f;
        float right = cx + width / 2.0f;
        float top = cy + height / 2.0f;
        float bottom = cy - height / 2.0f;

        // Background
        glBegin(GL_QUADS);
            glColor4f(0.02f, 0.25f, 0.2f, 0.9f); // Dark teal/emerald top
            glVertex2f(left, top);
            glVertex2f(right, top);
            glColor4f(0.01f, 0.05f, 0.12f, 0.95f); // Deep midnight blue bottom
            glVertex2f(right, bottom);
            glVertex2f(left, bottom);
        glEnd();

        // Sleek double gold border with pulse animation
        float tTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float pulse = 0.85f + 0.15f * std::sin(tTime * 3.5f);

        glLineWidth(3.0f);
        glColor4f(0.85f * pulse, 0.65f * pulse, 0.12f * pulse, 0.9f); // gold pulse
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        glLineWidth(1.0f);
        glColor4f(0.9f, 0.8f, 0.5f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left + 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, bottom + 5.0f);
            glVertex2f(right - 5.0f, top - 5.0f);
            glVertex2f(left + 5.0f, top - 5.0f);
        glEnd();

        // Title & Subtitle
        glColor4f(0.0f, 1.0f, 0.6f, 1.0f); // Bright Teal/Emerald Green
        drawCenteredString(GLUT_BITMAP_TIMES_ROMAN_24, "YOU WON", top - 45.0f, left, right);
        glColor4f(0.85f, 0.75f, 0.5f, 0.9f); // Soft gold subtext
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "You just killed Caine", top - 65.0f, left, right);

        // Divider
        glLineWidth(1.0f);
        glColor4f(0.85f, 0.65f, 0.12f, 0.4f);
        glBegin(GL_LINES);
            glVertex2f(left + 30.0f, top - 80.0f);
            glVertex2f(right - 30.0f, top - 80.0f);
        glEnd();

        // Options
        float startY = top - 135.0f;

        // Return to Menu
        glColor4f(0.0f, 0.9f, 0.5f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "[1]", left + 60.0f, startY);
        glColor4f(0.95f, 0.95f, 0.95f, 1.0f);
        drawString(GLUT_BITMAP_HELVETICA_18, "Return to Main Menu", left + 100.0f, startY);

        // Footer
        glColor4f(0.6f, 0.6f, 0.6f, 0.7f);
        drawCenteredString(GLUT_BITMAP_HELVETICA_12, "Press [1] to select an option", bottom + 25.0f, left, right);
    }
    else if (currentUIState == GAMEPLAY)
    {
        // HUD Hint during Gameplay
        float width = 140.0f;
        float height = 30.0f;
        float padding = 20.0f;
        float right = window.width - padding;
        float left = right - width;
        float top = window.height - padding;
        float bottom = top - height;

        glColor4f(0.08f, 0.08f, 0.12f, 0.6f);
        glBegin(GL_QUADS);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        glColor4f(0.85f, 0.65f, 0.12f, 0.5f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(left, bottom);
            glVertex2f(right, bottom);
            glVertex2f(right, top);
            glVertex2f(left, top);
        glEnd();

        glColor4f(0.0f, 0.9f, 0.5f, 0.9f);
        drawString(GLUT_BITMAP_HELVETICA_12, "[z] Pause Menu", left + 15.0f, bottom + 10.0f);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

void updateKeyStatesFromWindows()
{
    if (currentUIState != GAMEPLAY || (caineDeathSeqState >= 1 && caineDeathSeqState <= 3) || (caineIntroSeqState >= 1 && caineIntroSeqState <= 3))
    {
        keyStates['w'] = keyStates['W'] = false;
        keyStates['a'] = keyStates['A'] = false;
        keyStates['s'] = keyStates['S'] = false;
        keyStates['d'] = keyStates['D'] = false;
        keyStates[1] = false; // LBUTTON
        return;
    }
    keyStates['w'] = keyStates['W'] = (GetAsyncKeyState('W') & 0x8000) != 0;
    keyStates['a'] = keyStates['A'] = (GetAsyncKeyState('A') & 0x8000) != 0;
    keyStates['s'] = keyStates['S'] = (GetAsyncKeyState('S') & 0x8000) != 0;
    keyStates['d'] = keyStates['D'] = (GetAsyncKeyState('D') & 0x8000) != 0;

    keyStates[1] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
}

void myDisplayFunc(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get player position and scale for camera calculations
  float& kX = myvirtualworld.kinger.posX;
  float& kZ = myvirtualworld.kinger.posZ;
  const float kScale = myvirtualworld.kinger.uniformScale;

  static int lastTime = glutGet(GLUT_ELAPSED_TIME);
  int currentTime = glutGet(GLUT_ELAPSED_TIME);
  float deltaTime = (currentTime - lastTime) / 1000.0f;
  if (deltaTime > 0.1f) deltaTime = 0.1f;
  lastTime = currentTime;

  // Game state checks: Death & Win
  if (currentUIState == GAMEPLAY)
  {
      // Death Check: Kinger is dead and has finished his falling animation (1.0s delay)
      if (myvirtualworld.kinger.animation.isDead && myvirtualworld.kinger.animation.deathTimer >= 1.0f)
      {
          currentUIState = DEATH_SCREEN;
      }

      // Win Check: Caine is dead in Phase 3 (Only when in Test Arena)
      if (isTestArena && myvirtualworld.isCaineActive && myvirtualworld.caine.animation.isDead && myvirtualworld.caine.currentPhase == 3)
      {
          if (!isWinDelayed)
          {
              isWinDelayed = true;
              winDelayTimer = 0.0f;
              // Clear all Gloinks
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              // Clear all Caine projectiles
              for (int i = 0; i < myvirtualworld.caine.MAX_CAINE_PROJECTILES; i++)
              {
                  myvirtualworld.caine.projectiles[i].active = false;
              }
              // Clear Kinger bullets
              myvirtualworld.kinger.animation.isBulletActive = false;
          }

          winDelayTimer += deltaTime;
          if (winDelayTimer >= 5.0f)
          {
              currentUIState = WIN_SCREEN;
              isWinDelayed = false;
              myvirtualworld.audioManager.stopBackgroundMusic();
              myvirtualworld.audioManager.playSoundEffect("Audio\\SFX\\Win.wav");
          }
      }
  }

  // Cutscene State Machine Update
  if (currentUIState == GAMEPLAY && caineDeathSeqState > 0)
  {
      caineDeathSeqTimer += deltaTime;
      if (caineDeathSeqState == 1)
      {
          if (!hasStartedDeathSeq)
          {
              hasStartedDeathSeq = true;
              deathSeqCamStartEyeX = currentCameraEyeX;
              deathSeqCamStartEyeY = currentCameraEyeY;
              deathSeqCamStartEyeZ = currentCameraEyeZ;
              deathSeqCamStartTargetX = currentCameraTargetX;
              deathSeqCamStartTargetY = currentCameraTargetY;
              deathSeqCamStartTargetZ = currentCameraTargetZ;

              // Clear Gloinks and projectiles
              myvirtualworld.isGloinksActive = false;
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              for (int i = 0; i < myvirtualworld.caine.MAX_CAINE_PROJECTILES; i++)
              {
                  myvirtualworld.caine.projectiles[i].active = false;
              }
              myvirtualworld.kinger.animation.isBulletActive = false;
          }

          if (caineDeathSeqTimer >= 1.0f)
          {
              caineDeathSeqState = 2;
              caineDeathSeqTimer = 0.0f;
              caineDeathSeqTextLength = 0;
              caineDeathSeqTextTimer = 0.0f;
          }
      }
      else if (caineDeathSeqState == 2)
      {
          caineDeathSeqTextTimer += deltaTime;
          int targetLen = static_cast<int>(caineDeathSeqTextTimer / 0.08f);
          const std::string dialogueText = caineDeathSeqDialogue;
          if (targetLen > (int)dialogueText.length())
          {
              targetLen = (int)dialogueText.length();
          }
          caineDeathSeqTextLength = targetLen;

          if (caineDeathSeqTextLength >= (int)dialogueText.length())
          {
              caineDeathSeqState = 3;
              caineDeathSeqTimer = 0.0f;
          }
      }
      else if (caineDeathSeqState == 3)
      {
          if (caineDeathSeqTimer >= 5.0f)
          {
              caineDeathSeqState = 4;
              caineDeathSeqTimer = 0.0f;
              deathSeqCamStartEyeX = currentCameraEyeX;
              deathSeqCamStartEyeY = currentCameraEyeY;
              deathSeqCamStartEyeZ = currentCameraEyeZ;
              deathSeqCamStartTargetX = currentCameraTargetX;
              deathSeqCamStartTargetY = currentCameraTargetY;
              deathSeqCamStartTargetZ = currentCameraTargetZ;
          }
      }
      else if (caineDeathSeqState == 4)
      {
          if (caineDeathSeqTimer >= 5.0f)
          {
              caineDeathSeqState = 5;
              caineDeathSeqTimer = 0.0f;
              currentUIState = WIN_SCREEN;
              myvirtualworld.audioManager.stopBackgroundMusic();
              myvirtualworld.audioManager.playSoundEffect("Audio\\SFX\\Win.wav");
          }
      }
  }
  else
  {
      hasStartedDeathSeq = false;
  }

  // Intro Sequence State Machine Update
  if (currentUIState == GAMEPLAY && caineIntroSeqState > 0)
  {
      caineIntroSeqTimer += deltaTime;
      if (caineIntroSeqState == 1) // Zoom In (2.0s)
      {
          if (caineIntroSeqTimer >= 2.0f)
          {
              caineIntroSeqState = 2;
              caineIntroSeqTimer = 0.0f;
              caineIntroSeqTextLength = 0;
              caineIntroSeqTextTimer = 0.0f;
          }
      }
      else if (caineIntroSeqState == 2) // Scroll Dialogue
      {
          caineIntroSeqTextTimer += deltaTime;
          int targetLen = static_cast<int>(caineIntroSeqTextTimer / 0.08f);
          const std::string dialogueText = caineIntroSeqDialogue;
          if (targetLen > (int)dialogueText.length())
          {
              targetLen = (int)dialogueText.length();
          }
          caineIntroSeqTextLength = targetLen;

          if (caineIntroSeqTextLength >= (int)dialogueText.length())
          {
              caineIntroSeqState = 3;
              caineIntroSeqTimer = 0.0f;
          }
      }
      else if (caineIntroSeqState == 3) // Hold Dialogue (2.0s)
      {
          if (caineIntroSeqTimer >= 2.0f)
          {
              caineIntroSeqState = 4;
              caineIntroSeqTimer = 0.0f;
              
              // Capture camera position at the end of holding, so we can zoom out back from here
              introSeqCamStartEyeX = currentCameraEyeX;
              introSeqCamStartEyeY = currentCameraEyeY;
              introSeqCamStartEyeZ = currentCameraEyeZ;
              introSeqCamStartTargetX = currentCameraTargetX;
              introSeqCamStartTargetY = currentCameraTargetY;
              introSeqCamStartTargetZ = currentCameraTargetZ;
          }
      }
      else if (caineIntroSeqState == 4) // Zoom Out (1.5s)
      {
          if (caineIntroSeqTimer >= 1.5f)
          {
              caineIntroSeqState = 0; // Finish!
              caineIntroSeqTimer = 0.0f;
              
              // Enable Gloinks active updates and spawning for gameplay
              myvirtualworld.isGloinksActive = true;
              myvirtualworld.gloinks.initGloinks();

              if (difficultyLevel == 2)
              {
                  myvirtualworld.audioManager.playBackgroundMusic("Audio\\BGM\\Dark Souls III OST 10 - Vordt of the Boreal Valley.wav");
              }
          }
      }
  }
  else
  {
      hasStartedIntroSeq = false;
  }

 // Smoothly follow the player's Y position
 cameraTrackY += (myvirtualworld.kinger.posY - cameraTrackY) * CAMERA_FOLLOW_SPEED * deltaTime;

 // The camera target's Y position. It's based on the model's origin (18.7f above its base)
 // plus a shoulder offset. Both are now correctly scaled with the model.
 const float baseTargetY = cameraTrackY + (18.7f + CAMERA_TARGET_HEIGHT_OFFSET) * kScale;

 float rightX = std::cos(cameraYaw);
 float rightZ = -std::sin(cameraYaw);

 // The camera's look-at point, offset for an over-the-shoulder view.
 // This offset is now scaled to feel consistent regardless of player size.
  float targetX = kX + rightX * (CAMERA_SHOULDER_OFFSET * kScale);
  float targetZ = kZ + rightZ * (CAMERA_SHOULDER_OFFSET * kScale);

  float cosPitch = std::cos(cameraPitch);
  float sinPitch = std::sin(cameraPitch);

  // The camera's ideal displacement vector from target
  float vx = std::sin(cameraYaw) * cosPitch * (CAMERA_DISTANCE * kScale);
  float vy = (CAMERA_BASE_HEIGHT * kScale) + sinPitch * (CAMERA_DISTANCE * kScale);
  float vz = std::cos(cameraYaw) * cosPitch * (CAMERA_DISTANCE * kScale);

  // Retrieve skybox bounds for camera collision limit
  Vec3 skyMin, skyMax;
  myvirtualworld.environment.getSkyBoxBounds(skyMin, skyMax);
  float limitMinX = skyMin.x * boundaryScale;
  float limitMaxX = skyMax.x * boundaryScale;
  float limitMinZ = skyMin.z * boundaryScale;
  float limitMaxZ = skyMax.z * boundaryScale;
  if (limitMinX > limitMaxX) std::swap(limitMinX, limitMaxX);
  if (limitMinZ > limitMaxZ) std::swap(limitMinZ, limitMaxZ);

  float margin = 5.0f;
  float t = 1.0f;

  if (vx > 0.001f)
  {
      float tMaxX = ((limitMaxX - margin) - targetX) / vx;
      if (tMaxX < t) t = tMaxX;
  }
  else if (vx < -0.001f)
  {
      float tMinX = ((limitMinX + margin) - targetX) / vx;
      if (tMinX < t) t = tMinX;
  }

  if (vz > 0.001f)
  {
      float tMaxZ = ((limitMaxZ - margin) - targetZ) / vz;
      if (tMaxZ < t) t = tMaxZ;
  }
  else if (vz < -0.001f)
  {
      float tMinZ = ((limitMinZ + margin) - targetZ) / vz;
      if (tMinZ < t) t = tMinZ;
  }

  // Clamp camera above the floor plane (Y = -18.7f) with a small margin
  {
      const float floorY = -18.7f + margin;
      // Ray: eyeY = baseTargetY + t * vy. Solve for t when eyeY == floorY.
      if (vy < -0.001f)
      {
          float tFloor = (floorY - baseTargetY) / vy;
          if (tFloor > 0.0f && tFloor < t) t = tFloor;
      }
  }

  if (t < 0.1f) t = 0.1f;

  // The camera's final world position, zoomed in if close to the circus walls
  float eyeX = targetX + t * vx;
  float eyeY = baseTargetY + t * vy;
  float eyeZ = targetZ + t * vz;

  // Record normal target variables
  currentCameraTargetX = targetX;
  currentCameraTargetY = baseTargetY;
  currentCameraTargetZ = targetZ;

  float drawEyeX = eyeX;
  float drawEyeY = eyeY;
  float drawEyeZ = eyeZ;
  float drawTargetX = targetX;
  float drawTargetY = baseTargetY;
  float drawTargetZ = targetZ;

  if (caineDeathSeqState >= 1 && caineDeathSeqState <= 4)
  {
      ProjectCaine::Caine& boss = myvirtualworld.caine;
      Vec3 caineCenter = boss.getCaineWorldCenter();

      // Calculate normalized direction from State 1 start to Caine's center
      float sdx = caineCenter.x - deathSeqCamStartEyeX;
      float sdy = caineCenter.y - deathSeqCamStartEyeY;
      float sdz = caineCenter.z - deathSeqCamStartEyeZ;
      float slen = std::sqrt(sdx*sdx + sdy*sdy + sdz*sdz);
      float zoomDirX = 0.0f, zoomDirY = 0.0f, zoomDirZ = 1.0f;
      if (slen > 0.0f)
      {
          zoomDirX = sdx / slen;
          zoomDirY = sdy / slen;
          zoomDirZ = sdz / slen;
      }

      float destEyeX = caineCenter.x - zoomDirX * 45.0f;
      float destEyeY = caineCenter.y - zoomDirY * 45.0f;
      float destEyeZ = caineCenter.z - zoomDirZ * 45.0f;

      if (caineDeathSeqState == 1)
      {
          float t_val = caineDeathSeqTimer / 1.0f;
          if (t_val > 1.0f) t_val = 1.0f;
          float smoothT = t_val * t_val * (3.0f - 2.0f * t_val);

          drawEyeX = deathSeqCamStartEyeX + (destEyeX - deathSeqCamStartEyeX) * smoothT;
          drawEyeY = deathSeqCamStartEyeY + (destEyeY - deathSeqCamStartEyeY) * smoothT;
          drawEyeZ = deathSeqCamStartEyeZ + (destEyeZ - deathSeqCamStartEyeZ) * smoothT;

          // Interpolate view direction vector from starting camera direction to zoomDir
          float startDirX = deathSeqCamStartTargetX - deathSeqCamStartEyeX;
          float startDirY = deathSeqCamStartTargetY - deathSeqCamStartEyeY;
          float startDirZ = deathSeqCamStartTargetZ - deathSeqCamStartEyeZ;
          float startLen = std::sqrt(startDirX*startDirX + startDirY*startDirY + startDirZ*startDirZ);
          if (startLen > 0.0f)
          {
              startDirX /= startLen;
              startDirY /= startLen;
              startDirZ /= startLen;
          }
          else
          {
              startDirX = zoomDirX; startDirY = zoomDirY; startDirZ = zoomDirZ;
          }

          float curDirX = startDirX + (zoomDirX - startDirX) * smoothT;
          float curDirY = startDirY + (zoomDirY - startDirY) * smoothT;
          float curDirZ = startDirZ + (zoomDirZ - startDirZ) * smoothT;
          float curDirLen = std::sqrt(curDirX*curDirX + curDirY*curDirY + curDirZ*curDirZ);
          if (curDirLen > 0.0f)
          {
              curDirX /= curDirLen;
              curDirY /= curDirLen;
              curDirZ /= curDirLen;
          }

          float lookDist = 100.0f;
          drawTargetX = drawEyeX + curDirX * lookDist;
          drawTargetY = drawEyeY + curDirY * lookDist;
          drawTargetZ = drawEyeZ + curDirZ * lookDist;
      }
      else if (caineDeathSeqState == 2 || caineDeathSeqState == 3)
      {
          drawEyeX = destEyeX;
          drawEyeY = destEyeY;
          drawEyeZ = destEyeZ;

          drawTargetX = caineCenter.x;
          drawTargetY = caineCenter.y;
          drawTargetZ = caineCenter.z;
      }
      else if (caineDeathSeqState == 4)
      {
          float t_val = caineDeathSeqTimer / 1.0f;
          if (t_val > 1.0f) t_val = 1.0f;
          float smoothT = t_val * t_val * (3.0f - 2.0f * t_val);

          drawEyeX = deathSeqCamStartEyeX + (eyeX - deathSeqCamStartEyeX) * smoothT;
          drawEyeY = deathSeqCamStartEyeY + (eyeY - deathSeqCamStartEyeY) * smoothT;
          drawEyeZ = deathSeqCamStartEyeZ + (eyeZ - deathSeqCamStartEyeZ) * smoothT;

          // Interpolate view direction vector from zoomDir to normal follow look vector
          float fdx = targetX - eyeX;
          float fdy = baseTargetY - eyeY;
          float fdz = targetZ - eyeZ;
          float flen = std::sqrt(fdx*fdx + fdy*fdy + fdz*fdz);
          float followDirX = 0.0f, followDirY = 0.0f, followDirZ = 1.0f;
          if (flen > 0.0f)
          {
              followDirX = fdx / flen;
              followDirY = fdy / flen;
              followDirZ = fdz / flen;
          }

          float curDirX = zoomDirX + (followDirX - zoomDirX) * smoothT;
          float curDirY = zoomDirY + (followDirY - zoomDirY) * smoothT;
          float curDirZ = zoomDirZ + (followDirZ - zoomDirZ) * smoothT;
          float curDirLen = std::sqrt(curDirX*curDirX + curDirY*curDirY + curDirZ*curDirZ);
          if (curDirLen > 0.0f)
          {
              curDirX /= curDirLen;
              curDirY /= curDirLen;
              curDirZ /= curDirLen;
          }

          float lookDist = 100.0f;
          drawTargetX = drawEyeX + curDirX * lookDist;
          drawTargetY = drawEyeY + curDirY * lookDist;
          drawTargetZ = drawEyeZ + curDirZ * lookDist;
      }
  }

  if (caineIntroSeqState >= 1 && caineIntroSeqState <= 4)
  {
      ProjectCaine::Caine& boss = myvirtualworld.caine;
      Vec3 caineCenter = boss.getCaineWorldCenter();

      // Calculate normalized direction from intro start camera position to Caine's center
      float sdx = caineCenter.x - introSeqCamStartEyeX;
      float sdy = caineCenter.y - introSeqCamStartEyeY;
      float sdz = caineCenter.z - introSeqCamStartEyeZ;
      float slen = std::sqrt(sdx*sdx + sdy*sdy + sdz*sdz);
      float zoomDirX = 0.0f, zoomDirY = 0.0f, zoomDirZ = 1.0f;
      if (slen > 0.0f)
      {
          zoomDirX = sdx / slen;
          zoomDirY = sdy / slen;
          zoomDirZ = sdz / slen;
      }

      float destEyeX = caineCenter.x - zoomDirX * 45.0f;
      float destEyeY = caineCenter.y - zoomDirY * 45.0f;
      float destEyeZ = caineCenter.z - zoomDirZ * 45.0f;

      if (caineIntroSeqState == 1) // Zoom In (over 2.0s)
      {
          if (!hasStartedIntroSeq)
          {
              hasStartedIntroSeq = true;
              introSeqCamStartEyeX = eyeX;
              introSeqCamStartEyeY = eyeY;
              introSeqCamStartEyeZ = eyeZ;
              introSeqCamStartTargetX = targetX;
              introSeqCamStartTargetY = baseTargetY;
              introSeqCamStartTargetZ = targetZ;
          }

          float t_val = caineIntroSeqTimer / 2.0f;
          if (t_val > 1.0f) t_val = 1.0f;
          float smoothT = t_val * t_val * (3.0f - 2.0f * t_val);

          drawEyeX = introSeqCamStartEyeX + (destEyeX - introSeqCamStartEyeX) * smoothT;
          drawEyeY = introSeqCamStartEyeY + (destEyeY - introSeqCamStartEyeY) * smoothT;
          drawEyeZ = introSeqCamStartEyeZ + (destEyeZ - introSeqCamStartEyeZ) * smoothT;

          float startDirX = introSeqCamStartTargetX - introSeqCamStartEyeX;
          float startDirY = introSeqCamStartTargetY - introSeqCamStartEyeY;
          float startDirZ = introSeqCamStartTargetZ - introSeqCamStartEyeZ;
          float startLen = std::sqrt(startDirX*startDirX + startDirY*startDirY + startDirZ*startDirZ);
          if (startLen > 0.0f)
          {
              startDirX /= startLen;
              startDirY /= startLen;
              startDirZ /= startLen;
          }
          else
          {
              startDirX = zoomDirX; startDirY = zoomDirY; startDirZ = zoomDirZ;
          }

          float curDirX = startDirX + (zoomDirX - startDirX) * smoothT;
          float curDirY = startDirY + (zoomDirY - startDirY) * smoothT;
          float curDirZ = startDirZ + (zoomDirZ - startDirZ) * smoothT;
          float curDirLen = std::sqrt(curDirX*curDirX + curDirY*curDirY + curDirZ*curDirZ);
          if (curDirLen > 0.0f)
          {
              curDirX /= curDirLen;
              curDirY /= curDirLen;
              curDirZ /= curDirLen;
          }

          float lookDist = 100.0f;
          drawTargetX = drawEyeX + curDirX * lookDist;
          drawTargetY = drawEyeY + curDirY * lookDist;
          drawTargetZ = drawEyeZ + curDirZ * lookDist;
      }
      else if (caineIntroSeqState == 2 || caineIntroSeqState == 3) // Show Text box
      {
          drawEyeX = destEyeX;
          drawEyeY = destEyeY;
          drawEyeZ = destEyeZ;

          drawTargetX = caineCenter.x;
          drawTargetY = caineCenter.y;
          drawTargetZ = caineCenter.z;
      }
      else if (caineIntroSeqState == 4) // Zoom Out (over 1.5s)
      {
          float t_val = caineIntroSeqTimer / 1.5f;
          if (t_val > 1.0f) t_val = 1.0f;
          float smoothT = t_val * t_val * (3.0f - 2.0f * t_val);

          drawEyeX = introSeqCamStartEyeX + (eyeX - introSeqCamStartEyeX) * smoothT;
          drawEyeY = introSeqCamStartEyeY + (eyeY - introSeqCamStartEyeY) * smoothT;
          drawEyeZ = introSeqCamStartEyeZ + (eyeZ - introSeqCamStartEyeZ) * smoothT;

          float fdx = targetX - eyeX;
          float fdy = baseTargetY - eyeY;
          float fdz = targetZ - eyeZ;
          float flen = std::sqrt(fdx*fdx + fdy*fdy + fdz*fdz);
          float followDirX = 0.0f, followDirY = 0.0f, followDirZ = 1.0f;
          if (flen > 0.0f)
          {
              followDirX = fdx / flen;
              followDirY = fdy / flen;
              followDirZ = fdz / flen;
          }

          float curDirX = zoomDirX + (followDirX - zoomDirX) * smoothT;
          float curDirY = zoomDirY + (followDirY - zoomDirY) * smoothT;
          float curDirZ = zoomDirZ + (followDirZ - zoomDirZ) * smoothT;
          float curDirLen = std::sqrt(curDirX*curDirX + curDirY*curDirY + curDirZ*curDirZ);
          if (curDirLen > 0.0f)
          {
              curDirX /= curDirLen;
              curDirY /= curDirLen;
              curDirZ /= curDirLen;
          }

          float lookDist = 100.0f;
          drawTargetX = drawEyeX + curDirX * lookDist;
          drawTargetY = drawEyeY + curDirY * lookDist;
          drawTargetZ = drawEyeZ + curDirZ * lookDist;
      }
  }

  // Record camera position to global variables
  currentCameraEyeX = drawEyeX;
  currentCameraEyeY = drawEyeY;
  currentCameraEyeZ = drawEyeZ;

  // Calculate and record camera look direction to global variables
  float dx = drawTargetX - drawEyeX;
  float dy = drawTargetY - drawEyeY;
  float dz = drawTargetZ - drawEyeZ;
  float len = std::sqrt(dx * dx + dy * dy + dz * dz);
  if (len > 0.0f)
  {
      currentCameraDirX = dx / len;
      currentCameraDirY = dy / len;
      currentCameraDirZ = dz / len;
  }
  else
  {
      currentCameraDirX = -std::sin(cameraYaw) * std::cos(cameraPitch);
      currentCameraDirY = -std::sin(cameraPitch);
      currentCameraDirZ = -std::cos(cameraYaw) * std::cos(cameraPitch);
  }

  float shakeX = 0.0f;
  float shakeY = 0.0f;
  float shakeZ = 0.0f;
  if (screenShakeTimer > 0.0f)
  {
      shakeX = (-1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 2.0f)) * screenShakeIntensity;
      shakeY = (-1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 2.0f)) * screenShakeIntensity;
      shakeZ = (-1.0f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / 2.0f)) * screenShakeIntensity;
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(
      drawEyeX + shakeX,    drawEyeY + shakeY,        drawEyeZ + shakeZ,
      drawTargetX + shakeX, drawTargetY + shakeY, drawTargetZ + shakeZ,
      0.0f,    1.0f,        0.0f
  );

 glPushMatrix();

    worldaxis.draw();

    myvirtualworld.draw();

 glPopMatrix();

  // Draw 2D UI elements last
  drawCrosshair();
  drawHUD();
  drawMenuUI();

 glFlush();
 glutSwapBuffers();

  updateKeyStatesFromWindows();
  if (currentUIState == GAMEPLAY)
  {
      myvirtualworld.tickTime(cameraYaw, cameraPitch, keyStates);
  }
  glutPostRedisplay();
}

void myReshapeFunc(int width, int height)
{
 window.width  = width;
 window.height = height;
 glViewport(0, 0, width, height);
}

void myKeyboardFunc(unsigned char key, int x, int y)
{
  // Intercept keyboard actions in Death Screen
  if (currentUIState == DEATH_SCREEN)
  {
      if (key == '1')
      {
          myvirtualworld.startGame();
          currentUIState = GAMEPLAY;
          caineIntroSeqState = 1;
          caineIntroSeqTimer = 0.0f;
          caineIntroSeqTextLength = 0;
          caineIntroSeqTextTimer = 0.0f;
          hasStartedIntroSeq = false;
      }
      else if (key == '2')
      {
          myvirtualworld.resetGame();
          currentUIState = START_MENU;
      }
      glutPostRedisplay();
      return;
  }

  // Intercept keyboard actions in Win Screen
  if (currentUIState == WIN_SCREEN)
  {
      if (key == '1')
      {
          myvirtualworld.resetGame();
          currentUIState = START_MENU;
      }
      glutPostRedisplay();
      return;
  }

  // Toggle menu with 'z', 'Z' or ESC (27)
  if (key == 'z' || key == 'Z' || key == 27)
  {
      if (currentUIState == START_MENU)
      {
          if (key == 27)
          {
              exit(0);
          }
      }
      else if (currentUIState == GAMEPLAY)
      {
          currentUIState = PAUSE_MENU;
          glutPostRedisplay();
          return;
      }
      else if (currentUIState == PAUSE_MENU)
      {
          currentUIState = GAMEPLAY;
          glutPostRedisplay();
          return;
      }
  }

  // Intercept keyboard actions in Start Menu
  if (currentUIState == START_MENU)
  {
      switch (key)
      {
            case '1':
                isTestArena = false;
                myvirtualworld.startGame();
                currentUIState = GAMEPLAY;
                caineIntroSeqState = 1;
                caineIntroSeqTimer = 0.0f;
                caineIntroSeqTextLength = 0;
                caineIntroSeqTextTimer = 0.0f;
                hasStartedIntroSeq = false;
                break;
            case '2':
                isTestArena = false;
                myvirtualworld.debugEnvironment();
                currentUIState = GAMEPLAY;
                caineIntroSeqState = 1;
                caineIntroSeqTimer = 0.0f;
                caineIntroSeqTextLength = 0;
                caineIntroSeqTextTimer = 0.0f;
                hasStartedIntroSeq = false;
                break;
            case '3':
                isTestArena = true;
                myvirtualworld.resetGame();
                isTestArena = true; // resetGame sets isTestArena = false, so force it back
                myvirtualworld.isCaineActive = false;
                myvirtualworld.isGloinksActive = false;
                currentUIState = GAMEPLAY;
                break;
            case '4':
                difficultyLevel = (difficultyLevel + 1) % 3;
                break;
            case '0':
                exit(0);
                break;
      }
      glutPostRedisplay();
      return;
  }

  // Intercept keyboard actions in Pause Menu
  if (currentUIState == PAUSE_MENU)
  {
      switch (key)
      {
           case '1':
               if (isTestArena)
               {
                   myvirtualworld.resetGame();
                   isTestArena = true; // resetGame sets isTestArena = false, so force it back
                   myvirtualworld.isCaineActive = false;
                   myvirtualworld.isGloinksActive = false;
               }
               else if (myvirtualworld.isDebugMode)
               {
                   myvirtualworld.debugEnvironment();
                   caineIntroSeqState = 1;
                   caineIntroSeqTimer = 0.0f;
                   caineIntroSeqTextLength = 0;
                   caineIntroSeqTextTimer = 0.0f;
                   hasStartedIntroSeq = false;
               }
               else
               {
                   myvirtualworld.startGame();
                   caineIntroSeqState = 1;
                   caineIntroSeqTimer = 0.0f;
                   caineIntroSeqTextLength = 0;
                   caineIntroSeqTextTimer = 0.0f;
                   hasStartedIntroSeq = false;
               }
               currentUIState = GAMEPLAY;
               break;
          case '2':
              myvirtualworld.resetGame();
              currentUIState = START_MENU;
              break;
          case '0':
              currentUIState = GAMEPLAY;
              break;
      }
      glutPostRedisplay();
      return;
  }

  // Intercept keyboard actions during Caine death/intro sequence
  if ((caineDeathSeqState >= 1 && caineDeathSeqState <= 3) || (caineIntroSeqState >= 1 && caineIntroSeqState <= 3))
  {
      return;
  }

  keyStates[key] = true;

  switch (key)
  {
     // Jumping
     case ' ': myvirtualworld.kinger.jump(); break;

     // Roll Skill
     case 'c':
     case 'C':
         myvirtualworld.kinger.animation.castRollSkill(myvirtualworld.kinger.isGrounded);
         break;

     // Reload
     case 'r':
     case 'R':
         myvirtualworld.kinger.animation.castReload();
         break;

     // Toggle Hitbox Visibility
     case 'b':
     case 'B':
         showHitboxes = !showHitboxes;
         break;

     // Heal Skill
     case 'f':
     case 'F':
         myvirtualworld.kinger.animation.castHealSkill(myvirtualworld.kinger.currentHealth, myvirtualworld.kinger.maxHealth);
         break;

     // Test Damage/Hurt Visual Effect
     case 'h':
     case 'H':
         myvirtualworld.kinger.takeDamage(10);
         break;

     // Test Death Visual Effect
     case 'k':
     case 'K':
         myvirtualworld.kinger.takeDamage(100);
         break;

     // Test Caine Hurt Visual Effect
     case 'j':
     case 'J':
         myvirtualworld.caine.triggerHurt();
         break;

     // Caine Hand Animation Trigger & Gloink Hurt Triggers (Only active in Debug Environment)
      case '1':
          if (isTestArena)
          {
              myvirtualworld.environment.resetMeteors();
              myvirtualworld.isCaineActive = true;
              myvirtualworld.isGloinksActive = false;
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              myvirtualworld.caine.animation.isLayingDown = false;
              myvirtualworld.caine.animation.layDownFactor = 0.0f;
              myvirtualworld.caine.sweepActive = false;
              myvirtualworld.caine.animation.isLaughing = false;
              myvirtualworld.caine.currentHealth = myvirtualworld.caine.maxHealth;
              myvirtualworld.caine.animation.isDead = false;
              myvirtualworld.caine.animation.deathTimer = 0.0f;
              myvirtualworld.caine.testArenaSweepMode = false;
              myvirtualworld.caine.doctorStrangeState = 0;
          }
          else if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(0);
          break;
      case '2':
          if (isTestArena)
          {
              myvirtualworld.environment.resetMeteors();
              myvirtualworld.isCaineActive = true;
              myvirtualworld.isGloinksActive = false;
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              myvirtualworld.caine.animation.isLayingDown = true;
              myvirtualworld.caine.animation.isLaughing = true;
              myvirtualworld.caine.animation.layDownFactor = 1.0f;
              myvirtualworld.caine.sweepActive = true;
              myvirtualworld.caine.sweepDirection = rand() % 4;
              myvirtualworld.caine.sweepCurrentPos = (myvirtualworld.caine.sweepDirection == 0 || myvirtualworld.caine.sweepDirection == 2) ? -290.0f : 290.0f;
              myvirtualworld.caine.sweepTimer = 0.0f;
              myvirtualworld.caine.sweepInterval = 2.0f;
              myvirtualworld.caine.nextSweepDirection = rand() % 4;
              myvirtualworld.caine.currentHealth = myvirtualworld.caine.maxHealth;
              myvirtualworld.caine.animation.isDead = false;
              myvirtualworld.caine.animation.deathTimer = 0.0f;
              myvirtualworld.caine.testArenaSweepMode = true;
              myvirtualworld.caine.doctorStrangeState = 0;
          }
          else if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(1);
          break;
      case '3':
          if (isTestArena)
          {
              myvirtualworld.environment.resetMeteors();
              myvirtualworld.isCaineActive = false;
              myvirtualworld.isGloinksActive = true;
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              for (int i = 0; i < myvirtualworld.caine.MAX_CAINE_PROJECTILES; i++)
              {
                  myvirtualworld.caine.projectiles[i].active = false;
              }
              // Reset Caine to spawn
              myvirtualworld.caine.posX = 0.0f;
              myvirtualworld.caine.posY = 0.0f;
              myvirtualworld.caine.posZ = -120.0f;
              myvirtualworld.caine.doctorStrangeState = 0;
          }
          else if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(2);
          break;
      case '4':
          if (isTestArena)
          {
              myvirtualworld.environment.resetMeteors();
              myvirtualworld.isCaineActive = true;
              myvirtualworld.isGloinksActive = false;
              myvirtualworld.gloinks.animation.activeGloinks.clear();

              myvirtualworld.caine.doctorStrangeState = 1;
              myvirtualworld.caine.doctorStrangeTimer = 0.0f;
              myvirtualworld.caine.particleSpawnTimer = 0.0f;
              myvirtualworld.caine.currentHealth = myvirtualworld.caine.maxHealth;
              myvirtualworld.caine.animation.isDead = false;
              myvirtualworld.caine.animation.deathTimer = 0.0f;
              myvirtualworld.caine.animation.isLayingDown = false;
              myvirtualworld.caine.animation.layDownFactor = 0.0f;
              myvirtualworld.caine.sweepActive = false;
              myvirtualworld.caine.animation.isLaughing = false;
              myvirtualworld.caine.testArenaSweepMode = false;

              // Clear active Caine projectiles
              for (int i = 0; i < myvirtualworld.caine.MAX_CAINE_PROJECTILES; i++)
              {
                  myvirtualworld.caine.projectiles[i].active = false;
              }
              // Spawn initial teleport poof at current location
              myvirtualworld.caine.spawnTeleportPoof(myvirtualworld.caine.posX, myvirtualworld.caine.posY, myvirtualworld.caine.posZ);
          }
          else if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(3);
          break;
      case '5':
          if (isTestArena)
          {
              myvirtualworld.isCaineActive = true;
              myvirtualworld.isGloinksActive = false;
              myvirtualworld.gloinks.animation.activeGloinks.clear();
              myvirtualworld.caine.animation.isLayingDown = false;
              myvirtualworld.caine.animation.layDownFactor = 0.0f;
              myvirtualworld.caine.sweepActive = false;
              myvirtualworld.caine.animation.isLaughing = false;
              myvirtualworld.caine.currentHealth = myvirtualworld.caine.maxHealth;
              myvirtualworld.caine.animation.isDead = false;
              myvirtualworld.caine.animation.deathTimer = 0.0f;
              myvirtualworld.caine.testArenaSweepMode = false;
              myvirtualworld.caine.doctorStrangeState = 0;

              // Clear active Caine projectiles
              for (int i = 0; i < myvirtualworld.caine.MAX_CAINE_PROJECTILES; i++)
              {
                  myvirtualworld.caine.projectiles[i].active = false;
              }

              myvirtualworld.environment.isMeteorModeActive = true;
          }
          else if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(4);
          break;
     case '6':
         if (myvirtualworld.isDebugMode) myvirtualworld.gloinks.hurtGloink(5);
         break;
     case '7':
         if (myvirtualworld.isDebugMode) myvirtualworld.caine.animation.triggerShootingState();
         break;
     case '8':
         if (myvirtualworld.isDebugMode) myvirtualworld.caine.animation.toggleLayDown();
         break;
     case '9':
         if (myvirtualworld.isDebugMode) myvirtualworld.caine.animation.toggleLeanForward();
         break;
     case '0':
         if (myvirtualworld.isDebugMode) myvirtualworld.caine.triggerDeath();
         break;
  }
  glutPostRedisplay();
}

void myKeyboardUpFunc(unsigned char key, int x, int y)
{
 keyStates[key] = false;
}

void mySpecialFunc(int key, int x, int y)
{
 if (currentUIState != GAMEPLAY)
     return;

 switch (key)
 {
    case GLUT_KEY_UP   : cameraPitch -= CAMERA_KEY_TURN_INC;
                         if (cameraPitch < CAMERA_PITCH_MIN) cameraPitch = CAMERA_PITCH_MIN;
                         break;
    case GLUT_KEY_DOWN : cameraPitch += CAMERA_KEY_TURN_INC;
                         if (cameraPitch > CAMERA_PITCH_MAX) cameraPitch = CAMERA_PITCH_MAX;
                         break;
    case GLUT_KEY_LEFT : cameraYaw -= CAMERA_KEY_TURN_INC; break;
    case GLUT_KEY_RIGHT: cameraYaw += CAMERA_KEY_TURN_INC; break;

    case GLUT_KEY_HOME: myDataInit(); break;
 	case GLUT_KEY_F1  : setting.shadingMode = !setting.shadingMode;
                        if (setting.shadingMode)
                            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        else
                            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        break;
 	case GLUT_KEY_F2  : worldaxis.toggle(); break;
 	case GLUT_KEY_F3  : GLboolean lightingIsOn;
                        glGetBooleanv(GL_LIGHTING, &lightingIsOn);
                        if (lightingIsOn == GL_TRUE)
                            glDisable(GL_LIGHTING);
                        else
                            glEnable(GL_LIGHTING);
                        break;
 }
 glutPostRedisplay();
}

void myPassiveMotionFunc(int x, int y)
{
 int centerX = window.width  / 2;
 int centerY = window.height / 2;

  if (currentUIState != GAMEPLAY || (caineDeathSeqState >= 1 && caineDeathSeqState <= 3) || (caineIntroSeqState >= 1 && caineIntroSeqState <= 3))
  {
      if (x != centerX || y != centerY)
      {
          glutWarpPointer(centerX, centerY);
      }
      return;
  }

 if (x == centerX && y == centerY)
     return;

 int deltaX = x - centerX;
 int deltaY = y - centerY;

 cameraYaw -= deltaX * MOUSE_SENSITIVITY;

 cameraPitch += deltaY * MOUSE_SENSITIVITY;

 if (cameraPitch > CAMERA_PITCH_MAX) cameraPitch = CAMERA_PITCH_MAX;
 if (cameraPitch < CAMERA_PITCH_MIN) cameraPitch = CAMERA_PITCH_MIN;

 glutWarpPointer(centerX, centerY);

 glutPostRedisplay();
}

void myMouseFunc(int button, int state, int x, int y)
{
    if (currentUIState != GAMEPLAY || (caineDeathSeqState >= 1 && caineDeathSeqState <= 3) || (caineIntroSeqState >= 1 && caineIntroSeqState <= 3))
        return;

    switch (button)
    {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
            {
                myvirtualworld.kinger.animation.castGunSkill();

                keyStates[1] = true;
            }
            if (state == GLUT_UP)
            {
                keyStates[1] = false;
            }
            break;

        case GLUT_RIGHT_BUTTON:
            break;
    }
}

void myMotionFunc(int x, int y)
{
    myPassiveMotionFunc(x, y);
}

void myDataInit()
{
 window.title = "TCG6223 Computer Graphics";
 window.posX = 100;
 window.posY = 100;
 window.width  = 800;
 window.height = 500;

 world.rotateX  = 0.0;
 world.rotateY  = 0.0;
 world.rotateZ  = 0.0;
 world.posX     = 0.0;
 world.posY     = 0.0;
 world.posZ     = 0.0;
 world.scaleX   = 1.0;
 world.scaleY   = 1.0;
 world.scaleZ   = 1.0;

 viewer.eyeX    = 0.0;
 viewer.eyeY    = 0.0;
 viewer.eyeZ    = 40.0;
 viewer.centerX = 0.0;
 viewer.centerY = 0.0;
 viewer.centerZ = 0.0;
 viewer.upX     = 0.0;
 viewer.upY     = 1.0;
 viewer.upZ     = 0.0;
 viewer.zNear   = 0.1;
 viewer.zFar    = 10000.0;
 viewer.fieldOfView = 60.0;
 viewer.aspectRatio = static_cast<GLdouble> (window.width) / window.height;

 setting.posInc   = 1.0;
 setting.angleInc = 2.0;
 setting.mouseX   = 0;
 setting.mouseY   = 0;

 setting.mouseRightMode = false;
 setting.mouseLeftMode = false;

 setting.shadingMode = true;
}

void myViewingInit()
{
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(viewer.fieldOfView,
                viewer.aspectRatio,
                viewer.zNear,
                viewer.zFar);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 gluLookAt(viewer.eyeX,   viewer.eyeY,   viewer.eyeZ,
           viewer.centerX,viewer.centerY,viewer.centerZ,
           viewer.upX,    viewer.upY,    viewer.upZ );
}

void myLightingInit()
{
 static GLfloat  ambient[] = { 0.0f,  0.0f,  0.0f, 1.0f };
 static GLfloat  diffuse[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat specular[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat  specref[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat position[] = {10.0f, 10.0f, 10.0f, 1.0f };
  short shininess = 128;

 glDisable(GL_LIGHTING);
 glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
 glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
 glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
 glLightfv(GL_LIGHT0, GL_POSITION, position);
 glEnable(GL_LIGHT0);

 glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
 glEnable(GL_COLOR_MATERIAL);

  glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
  glMateriali(GL_FRONT, GL_SHININESS, shininess);

  glEnable(GL_NORMALIZE);
}

void enableConsoleANSI();
void initLoadingScreen();
void updateLoadingProgress(const std::string& action, const std::string& itemName);

int currentLoadStep = 0;
const int totalLoadSteps = 85;

#ifdef _WIN32
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

void enableConsoleANSI()
{
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void initLoadingScreen()
{
    enableConsoleANSI();
    currentLoadStep = 0;
    // Clear screen
    std::cout << "\033[2J\033[H";
    std::cout << "=================================================================\n";
    std::cout << "*                  THE AMAZING DIGITAL CIRCUS                   *\n";
    std::cout << "*                       L O A D I N G . . .                     *\n";
    std::cout << "=================================================================\n\n";
    std::cout << "  Progress: [>                                       ] 0%\n\n";
    std::cout << "  Initializing systems...\n\n";
    std::cout << "=================================================================\n";
    std::cout.flush();
}

void updateLoadingProgress(const std::string& action, const std::string& itemName)
{
    currentLoadStep++;
    if (currentLoadStep > totalLoadSteps) currentLoadStep = totalLoadSteps;

    float percent = (float)currentLoadStep / totalLoadSteps * 100.0f;
    int barWidth = 40;
    int pos = (int)(barWidth * ((float)currentLoadStep / totalLoadSteps));

    // Reset cursor to top-left (no screen flicker)
    std::cout << "\033[H";

    std::cout << "=================================================================\n";
    std::cout << "*                  THE AMAZING DIGITAL CIRCUS                   *\n";
    std::cout << "*                       L O A D I N G . . .                     *\n";
    std::cout << "=================================================================\n\n";

    std::cout << "  Progress: [";
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << (int)percent << "%\n\n";

    std::cout << "  " << action << ": \033[K" << itemName << "\n\n";
    std::cout << "=================================================================\n";
    std::cout.flush();
}

void myInit()
{
  myDataInit();

  glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
  // Get screen size first
  window.width = glutGet(GLUT_SCREEN_WIDTH) - 20;
  window.height = glutGet(GLUT_SCREEN_HEIGHT) - 80;

  // Create a large window that almost fills the screen
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(window.width, window.height);
  glutCreateWindow(window.title.c_str());

  glutDisplayFunc(myDisplayFunc);
  glutReshapeFunc(myReshapeFunc);
  glutKeyboardFunc(myKeyboardFunc);
  glutSpecialFunc(mySpecialFunc);
  glutMotionFunc(myMotionFunc);
  glutMouseFunc(myMouseFunc);
  glutPassiveMotionFunc(myPassiveMotionFunc); // mouse-look without button
  glutSetCursor(GLUT_CURSOR_NONE);            // hide cursor for immersive look

  glPointSize(4.0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glFrontFace(GL_CCW);
  glShadeModel (GL_SMOOTH);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_CULL_FACE);

  myViewingInit();

  myLightingInit();

  initLoadingScreen();
  myvirtualworld.init();

  // Clear screen and print control info when loading finishes
  #ifdef _WIN32
  system("cls");
  #else
  std::cout << "\033[2J\033[H";
  #endif
  myWelcome();

  myvirtualworld.resetGame();
}

void myWelcome()
{
  cout << "*****************************************************************\n";
  cout << "*                   TCG6223 Computer Graphics                   *\n";
  cout << "*                  FIST, Multimedia University                  *\n";
  cout << "*****************************************************************\n";
  cout << "| TPS Controls:                                                 |\n";
  cout << "|   <w>,<s>             => move Kinger forward / backward       |\n";
  cout << "|   <a>,<d>             => strafe Kinger left / right           |\n";
  cout << "|   <f>                 => cast heal skill                      |\n";
  cout << "|   <c>                 => roll                                 |\n";
  cout << "|   Mouse (move)        => rotate camera (yaw + pitch)          |\n";
  cout << "|   Arrow UP/DOWN       => camera pitch (keyboard fallback)     |\n";
  cout << "|   Arrow LEFT/RIGHT    => camera yaw   (keyboard fallback)     |\n";
  cout << "|   SPACE               => jump                                 |\n";
  cout << "|   HOME                => restore defaults                     |\n";
  cout << "|   ESC                 => exit                                 |\n";
  cout << "|                                                               |\n";
  cout << "|   F1  => toggle shading / wire-frame                          |\n";
  cout << "|   F2  => toggle axis rendering                                |\n";
  cout << "|   F3  => toggle lighting on / off                             |\n";
  cout << "|   <b> => toggle hitbox outlines on / off                      |\n";
  cout << "*****************************************************************\n";
  cout << "|                      H A V E   F U N  !!!                    |\n";
  cout << "*****************************************************************\n";
}

//--------------------------------------------------------------------
int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  myInit();

  glutMainLoop(); // Display everything and wait
}
//--------------------------------------------------------------------
