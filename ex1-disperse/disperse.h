
#define NO_MESSAGE 0    // Constant to declare no message content

// COLORS
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define YELLOW RGB(0,1,1)

// MOTION TYPE
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;


// STATES
typedef enum {
    MOVE_ONE,
    MOVE_TWO,
    MOVE_THREE
} move_state_t;


// GLOBAL VARIABLES
typedef struct 
{
  // Messages
  uint8_t new_message;     //Flag
  message_t transmit_msg;
  message_t received_msg;

  // State flag
  move_state_t current_state;

  // Time Management
  uint8_t last_time;

} USERDATA;




