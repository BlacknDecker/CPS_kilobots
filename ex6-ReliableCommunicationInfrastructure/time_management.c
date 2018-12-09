#include <kilombo.h>
#include "test_communication.h"		// Needs the main .h file due to the GLOBAL data structure
#include "time_management.h"		

extern USERDATA * mydata;


/******** TIME MANAGEMENT *******************/

void setupTimeManagement(){
  resetClock(COMMUNICATION_C);
  resetClock(BLINK_C);
  resetClock(DEFAULT_C);
}


uint8_t isInRange(uint8_t bottom, uint8_t top, clock_type_t c_type){
  // Calculate elapsed time 
  uint8_t elapsed_time;
  switch(c_type){
  	case COMMUNICATION_C: elapsed_time = kilo_ticks - mydata->communication_clock; break;
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
  	case COMMUNICATION_C: mydata->communication_clock = kilo_ticks; break;
    case BLINK_C: mydata->blink_clock = kilo_ticks; break;
    default: mydata->default_clock = kilo_ticks;
  }
}

/** Communication time management ***/

uint8_t isExpired(uint8_t * lifetime){
  if(*lifetime > DATA_LIFETIME){
  	return 1;	
  } else {
  	*lifetime += 1;
  	return 0;
  }
}

// Check if timeout is elapsed
uint8_t timeoutFires(){
  if(isInRange(0,TIMEOUT, COMMUNICATION_C)){
  	return 0;
  }else{
  	return 1;
  }
}
