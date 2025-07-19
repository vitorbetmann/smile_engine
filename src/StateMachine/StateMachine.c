// TODO(#7) trim state name when registering state and reject whitespace only
// TODO(#8) trim state name when changing states and reject whitespace only
// TODO(#9) create internal function to trim leading and trailing whitespace

// --------------------------------------------------
// Includes
// --------------------------------------------------

#include "StateMachine.h"
#include "../tests/StateMachine/StateMachineTest.h"
#include "StateMachineInternal.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// --------------------------------------------------

// --------------------------------------------------
static char* SM_Internal_TrimWhitespace(const char* str);

// --------------------------------------------------
// Defines
// --------------------------------------------------

#define SM_WARN(str, ...)                                                      \
  if (warningsEnabled) {                                                       \
    fprintf(stderr, "\033[33m[SMILE WARNING]\033[0m " str "\n",                \
            ##__VA_ARGS__);                                                    \
  }

#define SM_ERR(str, ...)                                                       \
  fprintf(stderr, "\033[31m[SMILE ERROR]\033[0m " str "\n", ##__VA_ARGS__)

// --------------------------------------------------
// Variables
// --------------------------------------------------
static bool warningsEnabled;
static int stateCount;
static StateTracker *tracker;
static bool canMalloc = true;

// --------------------------------------------------
// Functions
// --------------------------------------------------

bool SM_Init(void) {

  if (tracker) {
    SM_WARN("State Machine already initialized.");
    return false;
  }

  tracker = SM_Test_Malloc(sizeof(StateTracker));
  if (!tracker) {
    SM_ERR("Failed to allocate memory. State Machine not initialized.");
    return false;
  }
  tracker->stateMap = NULL;
  tracker->currState = NULL;
  stateCount = 0;

#if defined(SMILE_WARNINGS) && !defined(SMILE_RELEASE)
  SM_Internal_EnableWarnings(true);
#endif

  return true;
}

void SM_Internal_EnableWarnings(bool toggle) { warningsEnabled = toggle; }

bool SM_IsInitialized(void) { return tracker; }

bool SM_RegisterState(const char *name, void (*enterFn)(void *),
                      void (*updateFn)(float), void (*drawFn)(void),
                      void (*exitFn)(void)) {

  if (!tracker) {
    SM_ERR("State Machine not initialized.");
    return false;
  }

  if (!name) {
    SM_ERR("Can't register state with NULL name. No new state created.");
    return false;
  }

  if (strlen(name) == 0) {
    SM_ERR("Can't register state with empty name. No new state created.");
    return false;
  }

  if (SM_IsStateRegistered((char *)name)) {
    SM_WARN("A state called '%s' already exists. No new state created.", name);
    return false;
  }

  if (!enterFn && !updateFn && !drawFn && !exitFn) {
    SM_ERR("State '%s' has no valid functions. No new state created.", name);
    return false;
  }

  State *newState = malloc(sizeof(State));
  if (!newState) {
    SM_ERR("Failed to allocate memory. No new state '%s' created.", name);
    return false;
  }

  char *stateName = malloc(strlen(name) + 1);
  if (!stateName) {
    SM_ERR("Failed to allocate memory. No new state '%s' created.", name);
    free(newState);
    return false;
  }
  strcpy(stateName, name);

  newState->name = stateName;
  newState->enter = enterFn;
  newState->update = updateFn;
  newState->draw = drawFn;
  newState->exit = exitFn;

  StateMap *temp = malloc(sizeof(StateMap));
  if (!temp) {
    free((char *)newState->name);
    free(newState);
    SM_ERR("Failed to allocate memory. No new state '%s' created.", name);
    return false;
  }
  temp->state = newState;
  temp->name = newState->name;
  HASH_ADD_STR(tracker->stateMap, name, temp);

  stateCount++;

  return true;
}

bool SM_IsStateRegistered(const char* name) {
    if (!SM_IsInitialized()) return false;
    if (name == NULL) return false;
    // Trim whitespace from the input name
    char* trimmed_name = SM_Internal_TrimWhitespace(name);
    if (trimmed_name == NULL) return false;
    State* state = (State*)SM_Internal_GetState(trimmed_name);
    free(trimmed_name);
    return (state != NULL);
}

bool SM_ChangeStateTo(const char *name, void *args) {
    if (!SM_IsInitialized()) {
        SM_ERR("Can't change state. State Machine not initialized.");
        return false;
    }
    if (name == NULL) {
        SM_ERR("Can't change to state with NULL name. Current state not changed.");
        return false;
    }
    // Trim whitespace from the input name
    char* trimmed_name = SM_Internal_TrimWhitespace(name);
    if (trimmed_name == NULL) {
        SM_ERR("Can't change to state with empty/whitespace-only name.");
        return false;
    }
    // Use trimmed name for lookup
    State* target_state = (State*)SM_Internal_GetState(trimmed_name);
    free(trimmed_name);
    if (target_state == NULL) {
        SM_ERR("State '%s' not found.", name);
        return false;
    }
    // Change state logic (not recursive)
    if (tracker->currState && tracker->currState->exit) {
        tracker->currState->exit();
    }
    SM_Internal_SetCurrState(target_state);
    if (target_state->enter) {
        target_state->enter(args);
    }
    return true;
}

bool SM_Update(float dt) {
  if (!tracker) {
    SM_ERR("Not possible to update. State Machine not initialized.");
    return false;
  }

  State *currState = (State *)SM_Internal_GetCurrState();

  if (!currState) {
    SM_ERR("Not possible to update. No state set to current.");
    return false;
  }

  if (!currState->update) {
    SM_WARN("Not possible to update state: \"%s\". Update function is NULL.",
            currState->name);
    return false;
  }

  currState->update(dt);
  return true;
}

bool SM_Draw(void) {
  if (!tracker) {
    SM_ERR("Not possible to draw. State Machine not initialized.");
    return false;
  }

  State *currState = (State *)SM_Internal_GetCurrState();

  if (!currState) {
    SM_ERR("Not possible to draw. No state set to current.");
    return false;
  }

  if (!currState->draw) {
    SM_WARN("Not possible to draw state: \"%s\". Draw function is NULL.",
            currState->name);
    return false;
  }

  currState->draw();
  return true;
}

bool SM_Shutdown(void) {

  if (!tracker) {
    SM_ERR("Failed to shutdown. State Machine not initialized.");
    return false;
  }

  State *currState = (State *)SM_Internal_GetCurrState();
  if (currState && currState->exit) {
    currState->exit();
  }
  SM_Internal_SetCurrState(NULL);

  StateMap *el, *tmp;
  HASH_ITER(hh, tracker->stateMap, el, tmp) {
    HASH_DEL(tracker->stateMap, el);
    free((char *)el->state->name);
    free(el->state);
    free(el);
    stateCount--;
  }

  free(tracker);
  tracker = NULL;

  return true;
}

const char *SM_GetCurrStateName(void) {

  if (!tracker) {
    SM_ERR("Can't get current state name. State Machine not initialized.");
    return NULL;
  }

  return tracker->currState ? tracker->currState->name : NULL;
}

// --------------------------------------------------
// Functions - Internal
// --------------------------------------------------

bool SM_Internal_SetCurrState(const State *state) {
  if (!tracker) {
    SM_ERR("State Machine not initialized.");
    return false;
  }

  tracker->currState = state;
  return true;
}

const State *SM_Internal_GetCurrState(void) {

  if (!tracker) {
    SM_ERR("State Machine not initialized.");
    return NULL;
  }

  return tracker->currState;
}

const State *SM_Internal_GetState(const char *name) {

  if (!tracker) {
    SM_ERR("State Machine not initialized.");
    return NULL;
  }

  StateMap *sm;
  HASH_FIND_STR(tracker->stateMap, name, sm);
  return sm ? sm->state : NULL;
}

// Helper function to trim whitespace from both ends
static char* SM_Internal_TrimWhitespace(const char* str) {
    if (str == NULL) return NULL;
    // Skip leading whitespace
    while (*str && (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')) {
        str++;
    }
    if (*str == '\0') return NULL; // Empty string after trimming
    // Find end of string
    const char* end = str + strlen(str) - 1;
    // Skip trailing whitespace
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }
    // Allocate memory for trimmed string
    size_t len = end - str + 1;
    char* trimmed = (char*)malloc(len + 1);
    if (!trimmed) return NULL;
    strncpy(trimmed, str, len);
    trimmed[len] = '\0';
    return trimmed;
}

// --------------------------------------------------
// Functions - Tests
// --------------------------------------------------

const StateTracker *SM_Test_GetTracker(void) {

  if (!tracker) {
    SM_ERR("State Machine not initialized.");
    return NULL;
  }

  return tracker;
}

const int SM_Test_GetStateCount(void) { return stateCount; }

bool SM_Test_SetCanMalloc(bool toggle) {
  canMalloc = toggle;
  return toggle;
}

void *SM_Test_Malloc(size_t size) {
  if (!canMalloc) {
    return NULL;
  }
  return malloc(size);
}