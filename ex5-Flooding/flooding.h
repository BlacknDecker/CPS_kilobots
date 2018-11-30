// Constant to declare no message content
#define NO_MESSAGE 0    

// COLORS
#define OFF RGB(0,0,0)
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define WHITE RGB(3,3,3)
#define PURPLE RGB(1,0,1)
#define YELLOW RGB(2,2,0)
#define ORANGE RGB(2,1,0)

// CLOCKS TYPES
typedef enum {
  BLINK_C,
  MESSAGE_C,
  DEFAULT_C
} clock_type_t;


// STATES
typedef enum {
  INITIATOR,
  SLEEPING,
  DONE
} flooding_states_t;


// GLOBAL VARIABLES
typedef struct 
{
  // Messages
  uint8_t new_message;     //Flag
  message_t transmit_msg;
  uint8_t received_msg;
  uint8_t sender_id;

  // Time Management
  uint8_t default_clock;
  uint8_t blink_clock;  // Used only to blink!

  // States
  flooding_states_t state;    // My state

  // utilities
  uint8_t random_wait_time;

} USERDATA;




