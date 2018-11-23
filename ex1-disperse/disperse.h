#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884197169399375105820974944
#endif


// declare motion variable type
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;


// declare state variable type
typedef enum {
    MOVE_ONE,
    MOVE_TWO,
    MOVE_THREE
} move_state_t;


// declare variables
typedef struct 
{
  // Messages
  uint8_t new_message;
  message_t transmit_msg;
  // State flag
  uint8_t ready;
  move_state_t current_state; // K3 State
  // Time Management
  uint8_t last_time;   
  uint8_t elapsed_time;
} USERDATA;




