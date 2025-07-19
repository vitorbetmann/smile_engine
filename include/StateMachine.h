#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdbool.h>

// --------------------------------------------------
// Data types
// --------------------------------------------------
typedef struct StateTracker StateTracker;
typedef struct State State;

// --------------------------------------------------
// Prototypes
// --------------------------------------------------

/**
 * @brief Initializes the state machine.
 *
 * Allocates internal structures and prepares the system to register and run
 * states.
 *
 * @note This function is safe to call multiple times. If the machine is already
 * initialized, it returns false without error.
 *
 * @return true if initialized successfully, false if already initialized or if
 * memory allocation failed.
 * @author Vitor Betmann
 */
bool SM_Init(void);

/**
 * @brief Checks whether the state machine has been initialized.
 *
 * @return true if initialized, false otherwise.
 * @author Vitor Betmann
 */
bool SM_IsInitialized(void);

/**
 * @brief Registers a new named state with optional lifecycle callbacks.
 *
 * Each state must have a unique name. At least one lifecycle function must be
 * non-NULL.
 *
 * @param name     The name of the state (must be non-NULL and non-empty).
 * @param enterFn  Called when entering this state (can be NULL).
 * @param updateFn Called every update tick while this state is active (can be
 * NULL).
 * @param drawFn   Called every frame while this state is active (can be NULL).
 * @param exitFn   Called when exiting this state (can be NULL).
 *
 * @return true if registration succeeds, false otherwise.
 * @author Vitor Betmann
 */
bool SM_RegisterState(const char *name, void (*enterFn)(void *),
                      void (*updateFn)(float), void (*drawFn)(void),
                      void (*exitFn)(void));

/**
 * @brief Checks whether a state with the given name is registered.
 *
 * @param name The name of the state to check.
 * @return true if a state with the given name exists, false otherwise.
 * @author Vitor Betmann
 */
bool SM_IsStateRegistered(const char *name);

/**
 * @brief Switches to a different state by name, optionally passing arguments.
 *
 * Calls the current state's exit function (if any) and the new state's enter
 * one. Will exit and re-enter the same state if the requested name matches the
 * current state's name.
 *
 * @param name The name of the state to switch to.
 * @param args Optional arguments to pass to the new state's enter function.
 *
 * @return true if the state change succeeded, false otherwise.
 * @author Vitor Betmann
 */
bool SM_ChangeStateTo(const char *name, void *args);

/**
 * @brief Calls the update function of the current active state.
 *
 * If no update function is defined or if the machine is not initialized,
 * returns false.
 *
 * @param dt Delta time since last update.
 * @return true if update was successful, false otherwise.
 * @author Vitor Betmann
 */
bool SM_Update(float dt);

/**
 * @brief Calls the draw function of the current active state.
 *
 * If no draw function is defined or if the machine is not initialized, returns
 * false.
 *
 * @return true if draw was successful, false otherwise.
 * @author Vitor Betmann
 */
bool SM_Draw(void);

/**
 * @brief Shuts down the state machine and frees all internal memory.
 *
 * Calls the exit function of the current state (if defined) before cleanup.
 * After shutdown, all registered states are discarded and the tracker is reset.
 *
 * @return true if shutdown succeeded, false if the machine was not initialized.
 * @author Vitor Betmann
 */
bool SM_Shutdown(void);

/**
 * @brief Gets the name of the current active state.
 *
 * @return The name of the current state, or NULL if no state is active or the
 * machine is uninitialized.
 * @author Vitor Betmann
 */
const char *SM_GetCurrStateName(void);

// --------------------------------------------------
// Convenience macros
// --------------------------------------------------

/**
 * @brief Switches to a different state by name without arguments.
 * 
 * This is a convenience macro for the common case where no arguments
 * need to be passed to the enter function.
 * 
 * @param name The name of the state to switch to.
 */
#define SM_ChangeStateToSimple(name) SM_ChangeStateTo(name, NULL)

#endif