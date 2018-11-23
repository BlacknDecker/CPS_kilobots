#include <kilombo.h>
#include "disperse.h"

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

//Callback to Receive messages
void message_rx(message_t *m, distance_measurement_t *d) {
    mydata->new_message = 1;
}

// To setup a static message <------we should generalize to set different messages (maybe only integer mex for now)
void setup_message(uint8_t new_mex){
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.data[0] = new_mex;
  //finally, calculate a message check sum
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
}

//Callback to send a message
message_t *message_tx() 
{
  return &mydata->transmit_msg;
}


/******** TIME MANAGEMENT *******************/
uint8_t isInRange(uint8_t bottom, uint8_t top){
  mydata->elapsed_time = kilo_ticks - mydata->last_time;
  if(mydata->elapsed_time >= bottom && mydata->elapsed_time < top){
    return 1; //True
  } else {
    return 0; //False
  }
}

void resetClock(){
  mydata->last_time = kilo_ticks;
  return;
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

// fa un'azione per un certo tempo. suppone che il clock sia già resettato. Ritorna true quando il delay è terminato
uint8_t move(motion_t direction, uint8_t duration){
  if(isInRange(0,duration)){
    set_motion(direction);
    return 0;                 // Still in range, return false
  } else {
    set_motion(STOP);         // Movement ended, return true
    return 1;
  }
}


/******* KILO 2 MOVEMENT *******************/

void kiloTwoMovements(){
  switch(mydata->current_state){
    case MOVE_ONE:
      printf("STATE 1: %d\t", kilo_ticks);
      printf("Elapsed time: %d\n", mydata->elapsed_time);
      if(move(LEFT, 160)){                // Action in State 1                      
        printf("END STATE 1!\n");
        resetClock();                     // End state: reset clock and move to next state
        mydata->current_state = MOVE_TWO;
      }
      break;
    case MOVE_TWO:
      printf("STATE 2: %d\n", kilo_ticks);
      if(move(FORWARD, 200)){         // Action in State 2  (5 sec)      
        resetClock();                 // End state: reset clock and move to next state
        mydata->current_state = MOVE_THREE;
      }
      break;
    case MOVE_THREE:
      printf("STATE 3: %d\n", kilo_ticks);
      if(move(RIGHT, 96)){            // Action in State 3      
        resetClock();                 // End state: reset clock and move to initial state state
        mydata->current_state = MOVE_ONE;
      }
      break;
    default:
      printf("DEFAULT BEHAVIOR!\n");
      set_color(RGB(0,0,0));
  }
}


/******* SETUP,LOOP,MAIN *******************/

void loop() {
  /* MESSAGE EXCANGE PHASE */
    if (mydata->new_message && kilo_uid == 1){
      mydata->new_message = 0;
      kilo_message_tx = message_tx;
      //mydata->last_time = kilo_ticks; // Reset clock 
      mydata->ready = 1;
    } else if (mydata->new_message && kilo_uid == 0){
      // K0 received the ack
      mydata->new_message = 0;
      //kilo_message_tx = NULL;
      mydata->ready = 1;
    }
  /* START DO SOMETHING */
    if (kilo_uid == 0 || kilo_uid == 1){
      if (mydata->ready == 1){
        // K0
        if (kilo_uid == 0){
          blink(32, 64, RGB(1,1,0));
          set_motion(RIGHT); // K_0 CW
        } else { 
          // -- Blink -- K1
          if(isInRange(0,32)){  // PHASE OFF
            set_color(RGB(0,0,0));
            set_motion(FORWARD);
          } else if (isInRange(32, 64)){  // PHASE ON
            set_color(RGB(0,0,1));
            set_motion(LEFT); // K_1 CCW
          } else {
            resetClock(); // Reset clock
          }  
        }
      }
    }
    
    // KILO 2
    if (kilo_uid == 2){
      kiloTwoMovements();
    }
    
  
}


void setup()
{
  // Message variables
  mydata->new_message = 0;
  setup_message(123);  
  // State flag
  mydata->ready = 0;
  mydata->current_state = MOVE_ONE;
  // Time Management 
  mydata->last_time = kilo_ticks;
  mydata->elapsed_time = 0;
}


int main() {
    kilo_init();
    if (kilo_uid == 0){
      set_color(RGB(1,0,0));
      kilo_message_tx = message_tx;  // K0 Send GO
      kilo_message_rx = message_rx;  // K0 also receive
    }else if (kilo_uid == 1){
      set_color(RGB(0,1,0));
      kilo_message_rx = message_rx;  // k1 Listening
    } else if (kilo_uid == 2){
      set_color(RGB(10,0,0));
    }
    kilo_start(setup, loop);
    return 0;
}





