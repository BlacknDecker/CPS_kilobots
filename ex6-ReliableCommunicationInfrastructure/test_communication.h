
// COLORS
#define OFF RGB(0,0,0)
#define RED RGB(3,0,0)
#define GREEN RGB(0,3,0)
#define BLUE RGB(0,0,3)
#define WHITE RGB(3,3,3)
#define PURPLE RGB(1,0,1)
#define YELLOW RGB(2,2,0)
#define ORANGE RGB(2,1,0)

// DISTANCE
#define DANGER_D 45
#define WARNING_D 60
#define SAFE_D 70


/******* CONSTANTS ********************/
#define MAX_INT 255
#define MAX_NEIGHBOURS 10

// MESSAGE EXCHANGE
#define MAX_OUT_QUEUE 3
#define MAX_ATTEMPTS 3
#define TIMEOUT 64
#define DATA_LIFETIME 48



// MOTION TYPE
typedef enum {
  STOP,
  FORWARD,
  LEFT,
  RIGHT
} motion_t;


// CLOCKS TYPES
typedef enum {
  DEFAULT_C,
  BLINK_C,
  COMMUNICATION_C
} clock_type_t;


/*** Message Exchange States ***/
typedef enum{
  PINGING,
  SIMPLE_DELIVERY,
  EVALUATION,
  FAIL_A,
  WAIT_PING,
  FAIL_B,
  RCV_AVAILABLE,
  WAIT_ACK,
  ATTEMPTS_COUNT,
  FAIL_C,
  MESSAGE_DELIVERED
} message_ex_state_t;

/*** Message Sending Outcome ***/
// Return type for the sending function
typedef enum{
  FAILED,
  BUSY,
  IN_PROGRESS,
  DELIVERED,
  DONE
} message_sending_t;


/*** Message Payload ***/
typedef enum{
  NO_MESSAGE,
  PING,
  ACK,
  SOMEDATA
} message_payload_t;


// GLOBAL VARIABLES
typedef struct 
{
  /*** MESSAGES ***/
  message_ex_state_t mex_ex_state;        // Message Exchange State (to move in the finite state machine)
  uint8_t lifetime[MAX_NEIGHBOURS];       // Time from the last time a mex arrived (in kiloticks)
  uint8_t neighbours[MAX_NEIGHBOURS];     // neighbours[ID] == 1 if kilobot_ID is in range, else == 0
  uint8_t distance[MAX_NEIGHBOURS];       // if neighbourss[ID] is in range, show distance, else == MAX_INT
  uint8_t last_message[MAX_NEIGHBOURS];   // contains payload of the last mex received from the IDth bot
  message_t out_queue[MAX_OUT_QUEUE];     // Contains the messages to transmit
  message_t outgoing_mex;                 // Message to transmit
  message_t transmitting_mex;             // Message that i'm actually transmitting
  uint8_t recipient_id;                   // Recipient ID
  uint8_t attempts_counter;               // Counts the number of attempts in delivering a message
  // Flags
  uint8_t message_arrived;                // Updated if a new message arrived
  // Common Messages
  message_t ping_mex;                     // Ping Message 
  message_t ack_mex;                      // Ack Message

  /*** *** *** ***/

  /*** TIME ***/
  uint8_t default_clock;
  uint8_t blink_clock;              // Used only to blink!
  uint8_t communication_clock;      // Used for communication purposes

  /*** *** *** ***/

  // State

} USERDATA;




