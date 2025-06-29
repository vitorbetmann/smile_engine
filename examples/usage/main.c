// --------------------------------------------------
// Includes
// --------------------------------------------------
#include "../../include/smile.h"

// --------------------------------------------------
// Defines
// --------------------------------------------------

// --------------------------------------------------
// Data types
// --------------------------------------------------

// --------------------------------------------------
// Prototypes
// --------------------------------------------------

// --------------------------------------------------
// Variables
// --------------------------------------------------
extern State state_example;

// --------------------------------------------------
// Program main entry point
// --------------------------------------------------
bool is_running = true;
float dt;

int main(void) {
  sm_change_state(&state_example, NULL);

  while (is_running) {
    // Get dt
    sm_update(dt);
    sm_draw();
  }

  return 0;
}

// --------------------------------------------------
// Functions
// --------------------------------------------------
