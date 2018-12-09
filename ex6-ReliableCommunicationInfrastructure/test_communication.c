#include <kilombo.h>
#include "test_communication.h"
#include "main_prototypes.h"
#include "time_management.h"
#include "communication.h"

#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif

REGISTER_USERDATA(USERDATA)



/******* SETUP,LOOP,MAIN *******************/

void loop(){
  messageManagementRoutine();
  if(mydata->message_arrived){
    printf("%d > New message is arrived!\n", kilo_uid);
    mydata->message_arrived = 0;
  }
  if(kilo_uid == 0){          // SENDER
    if(isInRange(0,160, DEFAULT_C)){
      if(kilo_ticks%32 == 0){
        printf(".");
      }
    }else{
      switch(sendToOne(1, SOMEDATA)){
        case DELIVERED:
          printf(">> MESSAGE Sent!\n");
          resetClock(DEFAULT_C);
          break;
        case FAILED:
          printf(">> MESSAGE NOT Sent!\n");
          resetClock(DEFAULT_C);
          break;
        case BUSY:
          printf(">> SENDER BUSY\n");
          resetClock(DEFAULT_C);
          break;
        default: printf("."); //do nothing
      }
      /*
      printf("\n--- Send ---\n");
      sendToAll(SOMEDATA); //
      resetClock(DEFAULT_C);
      */
    }
  }
  fflush(NULL);
}


void setup(){
  setupTimeManagement();  // Time Variables
  setupMessageExchange(); // Message variables
  if(kilo_uid == 0){
    set_color(GREEN);
  }
  if(kilo_uid == 1){
    set_color(RED);
  }

}


int main() {
  kilo_init();
  kilo_start(setup, loop);
  return 0;
}





