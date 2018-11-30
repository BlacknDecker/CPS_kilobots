#include <kilombo.h>
#include "flooding.h"

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
/******* PROTOTIPES ********************/
void resetClock(clock_type_t c_type);
uint8_t isInRange(uint8_t bottom, uint8_t top, clock_type_t c_type);



/******* CONSTANTS ********************/
// static const uint8_t MAX_INT = 255; 
static const uint8_t WAIT_TIME = 160; // 5 sec


/******** RANDOM *******************************/

// initialize the seed of soft generator with a random number
void setupSeed(){
    rand_seed(rand_hard());
}



/******* MESSAGE EXCHANGING ******************/

// util function
void printTwoDigitNumber(uint8_t num){
	if(num < 10)
		printf("0");
	printf("%d", num);
}

// util function
void printThreeDigitNumber(uint8_t num){
	if(num < 100)
		printf("0");
	if(num < 10)
		printf("0");
	printf("%d", num);
}

// Prints a "log" message
void printMessage(){
	printf("Bot ");
	printTwoDigitNumber(kilo_uid);
	printf(" -> | sender: ");
	printTwoDigitNumber(mydata->sender_id);
	printf(" | msg: ");
	printThreeDigitNumber(mydata->received_msg);
	printf(" | state: ");
	printTwoDigitNumber(mydata->state);
	printf(" |\n");
}


// Generate a message
void setup_message(uint8_t payload){
  mydata->transmit_msg.type = NORMAL;                               // Message type
  uint8_t sender = kilo_uid << 4;									// Add ID on the first 4 bits (on the left)
  mydata->transmit_msg.data[0] = sender + payload;                  // Payload
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);    // Checksum
}

// Callback to send a message
message_t *message_tx(){
  return &mydata->transmit_msg;
}

// Get the sender uid
void getSenderID(message_t m){
	mydata->sender_id = m.data[0];		// Raw message
    mydata->sender_id >>= 4;			// Clear the payload data
}

// Get the message payload
void getMessagePayload(message_t m){
	mydata->received_msg = m.data[0];	// Raw message
	mydata->received_msg <<= 4;			// Clear the ID data
	mydata->received_msg >>= 4;			// Make the message readable again

}

// Callback to Receive messages
void message_rx(message_t *m, distance_measurement_t *d) {
    mydata->new_message = 1;
    getSenderID(*m);
    getMessagePayload(*m);
    // printMessage();
}



/******* STATE MANAGEMENT ********************/

// Called by loop
void initiatorManager(){
	if(isInRange(0,WAIT_TIME, DEFAULT_C)){		// Wait a random time before start
		set_color(GREEN);										// Do nothing while waiting
	}else{
		setup_message(GREEN);									// Send my color to neighbours
		// resetClock(DEFAULT_C);								// Reset Clock
		// mydata->state = DONE;									// Change state -> need to wait some time!
	}
	/*********************** CI VOGLIONO PIU' STATI!!!!!!! */
}



// Called by performAction (when receiving a message)
void sleepingManager(){
	setup_message(mydata->received_msg);						// When receiving a message, sends it to neighbours
	set_color(mydata->received_msg);							// Change my color
	// mydata->state = DONE;										// After sending a message i'm done!
	// setup_message(NO_MESSAGE);									// Stop sending messages
}


void doneManager(){
	printf("%d -> DONE!\n", kilo_uid);						   // Done -> do nothing!
}


void errorManager(){
	set_color(RED);
  	printf("ERROR! WRONG STATE!\n");
}


/******** PERFORM ACTION (Triggered by new mex) **********************/


// Perform an action reacting to a non-void message receive
void performAction(){
  printMessage();
  switch(mydata->state){
  	case INITIATOR: break;				// Should not receive messages!
  	case SLEEPING:  sleepingManager();  break;
  	case DONE:      break;				// Should not receive messages!
  	default: 		errorManager();
  }
}

// Read a message (if available) and perform an action
void readMessage(){
  mydata->new_message = 0;            //Reset Flag
  if(mydata->received_msg){   		  //If message not void
    performAction();                  // Perform action
  }
}


/******** TIME MANAGEMENT *******************/
uint8_t isInRange(uint8_t bottom, uint8_t top, clock_type_t c_type){
  // Calculate elapsed time 
  uint8_t elapsed_time;
  switch(c_type){
    case BLINK_C: elapsed_time = kilo_ticks - mydata->blink_clock; break;
    default: elapsed_time = kilo_ticks - mydata->default_clock;
  }
  // Check range
  if(elapsed_time >= bottom && elapsed_time < top){
    return 1; //True
  } else {
    return 0; //False
  }
}

void resetClock(clock_type_t c_type){
  switch(c_type){
    case BLINK_C: mydata->blink_clock = kilo_ticks; break;
    default: mydata->default_clock = kilo_ticks;
  }
}


/******* COMMANDS **************************/

void blink(uint8_t off_delay, uint8_t on_delay, uint8_t rgb_color){
  if(isInRange(0, off_delay, BLINK_C)){      // PHASE OFF
    set_color(OFF);
  } else if (isInRange(off_delay, off_delay+on_delay, BLINK_C)){  // PHASE ON
    set_color(rgb_color);
  } else {
    resetClock(BLINK_C); // Reset clock
  }
}



/******* Utility **************************/

// Assign initial state
void setupStates(){
	if(kilo_uid){
		mydata->state = SLEEPING;
	}else{
		mydata->state = INITIATOR; 				 // Kilo 0 is the Initiator
		mydata->random_wait_time = rand_soft();	 // To wait for a random time
	}
}

// Assign initial color
void assignInitialColor(){
	if(kilo_uid){
		set_color(OFF);
	}else{
		set_color(GREEN);
	}
}


/******* SETUP,LOOP,MAIN *******************/

void loop() {
  switch(mydata->state){
  	case INITIATOR: initiatorManager(); break;
  	case SLEEPING:  readMessage();      break;						//Activated by incoming message
  	case DONE:      break;
  	default: 		errorManager();
  }
}


void setup(){
  // Random
  setupSeed();
  // Message variables
  setup_message(NO_MESSAGE); 
  mydata->new_message = 0;
  kilo_message_tx = message_tx;
  kilo_message_rx = message_rx;
  mydata->received_msg = 0;
  // Time Management 
  resetClock(BLINK_C);
  resetClock(MESSAGE_C);
  resetClock(DEFAULT_C);
  // State
  setupStates();
  // Color
  assignInitialColor();
}


int main() {
  kilo_init();
  kilo_start(setup, loop);
  return 0;
}





