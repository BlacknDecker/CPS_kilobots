#include <kilombo.h>
#include "test_communication.h"
#include "time_management.h"
#include "communication.h"
#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif

extern USERDATA * mydata;	// Suppose you include this file after the main .h file



void setupMessageExchange(){
  mydata->mex_ex_state = PINGING;
  mydata->message_arrived = 0;
  mydata->attempts_counter = 0;
  // lifetime
  for(int i=0; i<MAX_NEIGHBOURS; i++){ mydata->lifetime[i] = 0; }
  // neighbours
  for(int i=0; i<MAX_NEIGHBOURS; i++){ mydata->neighbours[i] = 0; }
  // distance
  for(int i=0; i<MAX_NEIGHBOURS; i++){ mydata->distance[i] = MAX_INT; }
  // last message
  for(int i=0; i<MAX_NEIGHBOURS; i++){ mydata->last_message[i] = 0; }
  // out_queue
  for(int i=0; i<MAX_OUT_QUEUE; i++){ setup_message(&mydata->out_queue[i], NO_MESSAGE); }
  // outgoing message
  setup_message(&mydata->outgoing_mex, NO_MESSAGE);
  // transmitting
  setup_message(&mydata->transmitting_mex, NO_MESSAGE);
  // ping message
  setup_message(&mydata->ping_mex, PING);
  // ack message
  setup_message(&mydata->ack_mex, ACK);
  
  /** CALLBACKS **/
  kilo_message_tx = message_tx;
  kilo_message_rx = message_rx; 
}


/******* SERVICES *****************************/

/*** SEND TO ***/

// Send a message to a specific bot
message_sending_t sendToOne(uint8_t uid, message_payload_t payload){
  switch(mydata->mex_ex_state){
    case PINGING:                                         // Sender Available
          mydata->recipient_id = uid;                     // Set Recipient
          setup_message(&mydata->outgoing_mex, payload);  // Set message
          mydata->mex_ex_state = EVALUATION;              // Start sending algorithm
          printf("\n++ Reliable Sending Algorithm Started! ++\n"); // debug
          return IN_PROGRESS;                             // Returns feedback
    case SIMPLE_DELIVERY:                                 // Sender BUSY
          printf("BUSY: Sender is busy!\n");              // debug
          return BUSY;                                    // Returns feedback
    case FAIL_A:                                          // Fail
          printf("FAIL: Recipient Out of Range!\n");         // debug
          mydata->mex_ex_state = PINGING;                 // Algorithm ended
          return FAILED;                                  // Returns feedback
    case FAIL_B:                                          // Fail
          printf("FAIL: Receiver Busy!\n");               // debug
          mydata->mex_ex_state = PINGING;                 // Algorithm ended
          return FAILED;                                  // Returns feedback
    case FAIL_C:                                          // Fail
          printf("FAIL: Max number of attempts reached!\n"); // debug
          mydata->mex_ex_state = PINGING;                 // Algorithm ended
          return FAILED;                                  // Returns feedback
    case MESSAGE_DELIVERED:                               // Message delivered
          printf("DELIVERED: Message succesfully delivered!\n"); // debug
          mydata->mex_ex_state = PINGING;                 // Algorithm ended
          return DELIVERED;                               // Returns feedback
    default:
          printf("IN PROGRESS: The algorithm is working...\n"); // debug 
          return IN_PROGRESS;  
  }
}

// Send to all, without feedbacks:
message_sending_t sendToAll(message_payload_t payload){
  message_t tmp_msg;
  setup_message(&tmp_msg, payload);
  pushRequest(tmp_msg);
  return DONE;
}



/******* MESSAGE EXCHANGING ******************/


/*** MESSAGE MANAGEMENT ROUTINE ***/

// creare una funzione (da eseguire nel loop) che esegua la seguente routine:
// - Controlla se sono arrivati nuovi messaggi:
//      - In caso affermativo legge il nuovo messaggio //No, è spacchettato in automatico
// - Controlla se sto inviando un messaggio:
//      - In caso affermativo prosegue nell'algoritmo di invio
// - Altrimenti controlla se ci sono messaggi da inviare
//      - In caso affermativo avvia l'algoritmo di invio
// - Controlla i dati:
//      - Se un dato ha raggiunto la data di scadenza lo resetta.
void messageManagementRoutine(){
  if(mydata->mex_ex_state == PINGING){  // Just pinging
    uploadWork();                       // Check if i have a message to send in the queue and in affirmative case take it
  }
  sendingAlgorithm();                   // Perform a step in the sending algorithm, based on the state
  checkExpiredData();                   // Check if data are still consistent
}


void uploadWork(){
  for(int i=0; i<MAX_OUT_QUEUE; i++){
    if(getMessagePayload(mydata->out_queue[i])){  // New message in the queue
      // printf("> %d UPLOADING WORK!\n", kilo_uid);
      mydata->outgoing_mex = mydata->out_queue[i];  // Move mex in the current working mex
      setup_message(&mydata->out_queue[i], NO_MESSAGE); // Remove message from the queue
      resetClock(COMMUNICATION_C);                       //Reset Clock
      mydata->mex_ex_state = SIMPLE_DELIVERY;           // Send the message in a simpler way (not reliable)
      mydata->transmitting_mex = mydata->outgoing_mex;  // Put the message in the transmitting variable
      return;                                           // Exit from the function
    }
  }
}

void sendingAlgorithm(){
  switch(mydata->mex_ex_state){
    case PINGING: pinging(); break;
    case EVALUATION: evaluation(); break;         // This state is activated by the sendTo function!
    case WAIT_PING: waitPing(); break;
    case RCV_AVAILABLE: rcvAvailable(); break;
    case WAIT_ACK: waitAck(); break;
    case ATTEMPTS_COUNT: attemptsCount(); break;
    case MESSAGE_DELIVERED: break;
    case SIMPLE_DELIVERY: simpleDelivery(); break;
    default: pinging();
  }
}

/*** Sending Algorithm Util Functions ***/

void pinging(){
  // printf("> %d PINGING\n", kilo_uid);
  mydata->attempts_counter = 0;                 // Reset Counter
  mydata->transmitting_mex = mydata->ping_mex;  // Change the transmitting mex
}

void evaluation(){
  // printf("> %d EVALUATING\n", kilo_uid);
  uint8_t dest = mydata->recipient_id;
  printNeighbourList();
  if(mydata->neighbours[dest]){ //Recipient is in the neighbour list
    if(mydata->last_message[dest] == PING){ // If last message was a ping
      mydata->mex_ex_state = RCV_AVAILABLE; // The recipient is available
    } else {
      resetClock(COMMUNICATION_C);          // Reset timer
      mydata->mex_ex_state = WAIT_PING;     // The recipient is busy
    } 
  } else { // Recipient is out of range
    mydata->mex_ex_state = FAIL_A;          // Recipient is out of range
  }
}

void waitPing(){
  // printf("> %d WAIT PING\n", kilo_uid);
  if(timeoutFires()){                       // Timeout elapsed
    mydata->mex_ex_state = FAIL_B;          // Recipient is busy
  } else {
    if(mydata->last_message[mydata->recipient_id] == PING){ // Received a ping
      mydata->mex_ex_state = RCV_AVAILABLE; // The recipient is available
    }
  }
}

void rcvAvailable(){
  // printf("> %d RECEIVER IS AVAILABLE\n", kilo_uid);
  mydata->transmitting_mex = mydata->outgoing_mex;  // Start transmitting the message
  resetClock(COMMUNICATION_C);              // Reset Clock
  mydata->mex_ex_state = WAIT_ACK;          // Change state
}

void waitAck(){
  // printf("> %d WAITING ACK\n", kilo_uid);
  if(timeoutFires()){                       // Timeout elapsed
    mydata->mex_ex_state = ATTEMPTS_COUNT;  // Check the attempts counter
  } else {
    if(mydata->last_message[mydata->recipient_id] == ACK){ // Received the ACK
      mydata->mex_ex_state = MESSAGE_DELIVERED; // The recipient confirmed
    }
  }
}

void attemptsCount(){
  // printf("> %d ATTEMPTS COUNT\n", kilo_uid);
  if(mydata->attempts_counter < MAX_ATTEMPTS){  // If i can have another try
    mydata->attempts_counter += 1;              // Increase counter
    mydata->mex_ex_state = EVALUATION;          // Move back to Evaluation
  }else{
    mydata->mex_ex_state = FAIL_C;              // Max attempts reached
  }
}

void simpleDelivery(){
  // printf("> %d SIMPLE DELIVERY\n", kilo_uid);
  if(timeoutFires()){               // Send until timeout expires
    mydata->mex_ex_state = PINGING; // When expired go back to pinging
  }
}


/*** *** ***/

void checkExpiredData(){
  for(int i=0; i<MAX_NEIGHBOURS; i++){
    if(mydata->neighbours[i]){              // Check only neighbours
      if(isExpired(&mydata->lifetime[i])){   // If data have lifetime has reached the end -> reset!
        if(kilo_uid==0){printf("> %d: My neighbour %d is expired -> lifetime: %d\n", kilo_uid, i, mydata->lifetime[i]);} // DEBUG
        mydata->neighbours[i]   = 0;
        mydata->distance[i]     = MAX_INT;
        mydata->last_message[i] = 0;
        if(kilo_uid==0){printf("Updated neighbours list: ");}                         // DEBUG
        if(kilo_uid==0){printNeighbourList();}                                        // DEBUG          
      }
    }
  }
}


/*** SEND ***/

// Generate a message
void setup_message(message_t * msg, message_payload_t payload){
  msg->type = NORMAL;                        // Message type
  uint8_t sender = kilo_uid << 4;						 // Add ID on the first 4 bits (on the left)
  msg->data[0] = sender + (uint8_t)payload;  // Payload
  msg->crc = message_crc(msg);               // Checksum
}


// Callback to send a message
message_t * message_tx(){  
  return &mydata->transmitting_mex;
}


/*** RECEIVE **/

// Get the sender uid
uint8_t getSenderID(message_t m){
	uint8_t snd_id = m.data[0];		// Raw message
  snd_id >>= 4;			            // Clear the payload data
  return snd_id;
}

// Get the message payload
uint8_t getMessagePayload(message_t m){
	uint8_t payload = m.data[0];	// Raw message
	payload <<= 4;			          // Clear the ID data
	payload >>= 4;			          // Make the message readable again
  return payload;
}

// Callback to Receive messages
void message_rx(message_t *m, distance_measurement_t *d){
  /* Store message in the archive */
  uint8_t snd = getSenderID(*m);                      // Get Sender ID
  mydata->lifetime[snd] = 0;                          // Reset lifetime
  mydata->neighbours[snd] = 1;                        // Update neighbours
  mydata->distance[snd] = estimate_distance(d);       // Update distance
  uint8_t tmp_payload = getMessagePayload(*m);
  if(isNewMessage(snd, tmp_payload)){                 // If is a new message, notify
    mydata->last_message[snd] = tmp_payload;          // Get Payload
    notifyNewMessage(snd);                            // Notify only if is a relevant message
  }
  // printMessageArchive();                           // DEBUG only
}

// To avoid duplicates
uint8_t isNewMessage(uint8_t sender, uint8_t payload){
  if(payload == mydata->last_message[sender]){
    return 0;
  }else{
    return 1;
  }
}

void notifyNewMessage(uint8_t sender){
  if(mydata->last_message[sender] != NO_MESSAGE){
    if(mydata->last_message[sender] != PING){
      if(mydata->last_message[sender] != ACK){
        mydata->message_arrived = 1;                  // Set flag to notify
        pushRequest(mydata->ack_mex);                 // Push a message in the message queue
      }
    }
  }
}

void pushRequest(message_t mex){
  for(int i=0; i<MAX_OUT_QUEUE; i++){
    if(getMessagePayload(mydata->out_queue[i]) == 0){    // If the slot is free
      mydata->out_queue[i] = mex;                     // Insert message
      // printf("> %d PUSHING REQUEST\n", kilo_uid);
      printMessageArchive();
      return;                                         // end
    }
  }
  // If no free slot found, ignore request
}


/******** PRETTY PRINT ************************/
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
void printMessage(uint8_t sender){
  printf("Bot ");
  printTwoDigitNumber(kilo_uid);
  printf(" -> | sender: ");
  printTwoDigitNumber(sender);
  printf(" | msg: ");
  printThreeDigitNumber(mydata->last_message[sender]);
//  printf(" | state: ");
//  printTwoDigitNumber(mydata->state);
  printf(" |\n");
}

// Prints all non-void messages
void printMessageArchive(){
  printf("+++ MESSAGES +++\n");
  for(int i=0; i<MAX_NEIGHBOURS; i++){
    if(mydata->last_message[i] != 0){
      printMessage(i);
    }
  }
  printf("+++ +++ +++\n");
  fflush(NULL);
}

// Debug
void printNeighbourList(){
  printf("NEIGHBOURS: ");
  for(int i=0; i<MAX_NEIGHBOURS; i++){
    if(mydata->neighbours[i]){
      printf("%d->life:%d | ", i, mydata->lifetime[i]);
    }
  }
  printf("\n");
  fflush(NULL);
}
