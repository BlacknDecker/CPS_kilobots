#include <kilombo.h>
#include "circle.h"

#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif

REGISTER_USERDATA(USERDATA)

/*
** 
*/

/******* MOVEMENT *********************/

void set_motion(motion_t new_motion)
{
  switch(new_motion) {
  case STOP:
    set_motors(0,0);
    break;
  case FORWARD:
    set_motors(kilo_turn_left, kilo_turn_right);
    break;
  case LEFT:
    set_motors(kilo_turn_left, 0); 
    break;
  case RIGHT:
    set_motors(0, kilo_turn_right); 
    break;
  }
}


/******* MESSAGE EXCHANGING ******************/

// Generate a message
void setup_message(uint8_t new_mex){
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.data[0] = new_mex;
  //finally, calculate a message check sum
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
}

// Callback to send a message
message_t *message_tx(){
  return &mydata->transmit_msg;
}

// Callback to Receive messages
void message_rx(message_t *m, distance_measurement_t *d) {
    mydata->new_message = 1;
    mydata->received_msg = *m;
}


/******** PERFORM ACTION **********************/

// Perform an action reacting to a non-void message receive
void performAction(){
  switch(mydata->received_msg.data[0]){
    case 1: set_color(RED); break;
    case 2: set_color(GREEN); break;
    case 3: set_color(BLUE); break;
    case 4: set_color(YELLOW); break;
    default: set_color(RGB(0,0,0));
  }
}

// Read a message (if available) and perform an action
void readMessage(){
  mydata->new_message = 0;            //Reset Flag
  if(mydata->received_msg.data[0]){   //If message not void
    performAction();                  // Perform action
  }
}


/******** TIME MANAGEMENT *******************/
uint8_t isInRange(uint8_t bottom, uint8_t top){
  uint8_t elapsed_time = kilo_ticks - mydata->last_time;
  if(elapsed_time >= bottom && elapsed_time < top){
    return 1; //True
  } else {
    return 0; //False
  }
}

void resetClock(){
  mydata->last_time = kilo_ticks;
}


/******* COMMANDS **************************/

void blink(uint8_t off_delay, uint8_t on_delay, uint8_t rgb_color){
  if(isInRange(0,off_delay)){      // PHASE OFF
    set_color(RGB(0,0,0));
  } else if (isInRange(off_delay, off_delay+on_delay)){  // PHASE ON
    set_color(rgb_color);
  } else {
    resetClock(); // Reset clock
  }
}

// Moves for a given amount of time (ticks). 
// Returns true when the movement is finished, otherwise false.
uint8_t move(motion_t direction, uint8_t duration){
  if(isInRange(0,duration)){
    set_motion(direction);
    return 0;                 // Still moving, return false
  } else {
    set_motion(STOP);         // Movement ended, return true
    return 1;
  }
}

// Moves in a circle. Radius 1-4
// Direction MUST be left or right!
void moveInCircle(uint8_t radius, motion_t direction){
  uint8_t duration = (radius * 32) - 1;
  if(isInRange(0,duration)){                      
    set_motion(FORWARD);
    return;
  } else if(isInRange(duration, 2*duration)) {
    set_motion(direction);
    return;
  } else {
    resetClock();
  }
}



/******* Utility **************************/

//Assign initial color
void assignInitialColor(){
  switch (kilo_uid){
    case 0: set_color(RGB(1,0,0)); break;
    case 1: set_color(RGB(1,1,0)); break;
    case 2: set_color(RGB(1,1,1)); break;
    case 3: set_color(RGB(1,0,1)); break;
    default: set_color(RGB(0,0,0));
  }
}



/******* SETUP,LOOP,MAIN *******************/

void loop() {
  // 1 - Read message (if available)
  if(mydata->new_message){
    readMessage();
  }
  // 2 - Moves
  switch (kilo_uid){
    case 0: moveInCircle(2, LEFT); break;
    case 1: moveInCircle(2, LEFT); break;
    case 2: moveInCircle(2, LEFT); break;
    case 3: moveInCircle(2, LEFT); break;
    default: set_color(RGB(0,0,0));
  }
}


void setup()
{
  // Message variables
  mydata->new_message = 0;
  kilo_message_tx = message_tx;
  kilo_message_rx = message_rx;
  setup_message(kilo_uid + 1);  
  // State flag
  mydata->current_state = MOVE_ONE;
  // Time Management 
  resetClock();
  // Color
  assignInitialColor();
}


int main() {
    kilo_init();
    kilo_start(setup, loop);
    return 0;
}





