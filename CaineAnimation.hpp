#ifndef CAINE_ANIMATION_HPP
#define CAINE_ANIMATION_HPP

/**
 * CaineAnimation
 * Manages the animation state, timers, and interpolation factors for the boss character Caine.
 * 
 * This class tracks various animation states including idle hovering/breathing, jaw flapping,
 * staff swaying, shooting sequence, laying down, leaning forward, and death/respawn timers.
 */
class CaineAnimation
{
public:
    // --- Idle Animation State ---
    float idleTimer;        ///< Elapsed time accumulator for idle animations
    float hoverOffset;      ///< Vertical bobbing offset calculated via sine wave
    float bodyTiltAngle;    ///< Subtle breathing roll tilt applied to the body
    float jawFlapAngle;     ///< Rotational angle for jaw movement during idle state
    float staffSwayAngle;   ///< Swaying angle of the staff (deprecated/breathing sync)
    float mouthOpenFactor;  ///< Interpolation factor [0.0, 1.0] for mouth opening

    // Shooting Animation State 
    bool isShootingState;   // Flag indicating if the shooting animation is active
    float shootingTimer;    // Timer tracking the progress of the shooting sequence [0.0, 1.0]

    // Lay Down Animation State
    bool isLayingDown;      // Flag indicating if Caine is laying down
    float layDownFactor;    // Interpolation factor [0.0, 1.0] for the laying down transition

    // Lean Forward Animation State
    bool isLeaningForward;  // Flag indicating if Caine is leaning forward (45 degrees)
    float leanForwardFactor; // Interpolation factor [0.0, 1.0] for the leaning forward transition

    // Death & Respawn State
    bool isDead;            // Flag indicating if Caine is dead
    float deathTimer;       // Timer tracking the death and respawn sequence [0.0, 2.0]

    // Laughing State
    bool isLaughing;        // Flag indicating if Caine is laughing

    // Hurt/Damage State Variables
    bool isHurt;            // Flag indicating if Caine is hurt and flashing
    float hurtTimer;        // Timer tracking hurt duration

    /**
     * Constructor that initializes all animation states, timers, and factors to their defaults.
     */
    CaineAnimation();

    /**
     * Updates the idle animation states (hover offset, body tilt, and jaw flap) using delta time.
     * dt The elapsed frame time in seconds.
     */
    void updateIdleState(float dt);

    /**
     * Triggers the shooting animation sequence, resetting the sequence timer.
     */
    void triggerShootingState();

    /**
     * Updates the shooting sequence timer over time.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateShootingState(float deltaTime);

    /**
     * Toggles the laying down animation state.
     */
    void toggleLayDown();

    /**
     * Interpolates the laying down transition factor based on the current state.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateLayDown(float deltaTime);

    /**
     * Toggles the lean forward animation state.
     */
    void toggleLeanForward();

    /**
     * Interpolates the lean forward transition factor based on the current state.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateLeanForward(float deltaTime);

    /**
     * Updates the damage stun flashing timer.
     * deltaTime The elapsed frame time in seconds.
     */
    void updateHurtState(float deltaTime);

    /**
     * Activates the hurt state, interrupting active shooting animations.
     */
    void triggerHurt();

    /**
     * Applies the multi-stage shooting rotation transformations to the current OpenGL matrix stack.
     * 
     * This function performs a 3-stage sequence:
     * - Stage 1 (0% to 30%): Hand raise and roll back.
     * - Stage 2 (30% to 70%): Hand points forward.
     * - Stage 3 (70% to 100%): Hand returns to the starting posture.
     */
    void applyShootingAnimation() const;
};

#endif // CAINE_ANIMATION_HPP
