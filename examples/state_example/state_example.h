#ifndef STATE_EXAMPLE_H
#define STATE_EXAMPLE_H
// --------------------------------------------------
// Includes
// --------------------------------------------------
#include "../../include/smile.h"

// --------------------------------------------------
// Other defines
// --------------------------------------------------

// --------------------------------------------------
// Data types
// --------------------------------------------------

// --------------------------------------------------
// Prototypes
// --------------------------------------------------
void state_example_enter(void *args);
void state_example_update(float dt);
void state_example_draw(void);
void state_example_exit(void);

// --------------------------------------------------
// Variables
// --------------------------------------------------
extern State state_example;

#endif