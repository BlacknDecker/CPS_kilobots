

/*** SERVICES PROVIDED ***/
message_sending_t sendToOne(uint8_t uid, message_payload_t payload); // Send a message to a specific bot (gives a feedback)
message_sending_t sendToAll(message_payload_t payload);


/*******PROTOTYPEs*************/
void setupMessageExchange();		// Setups all the message exchange related variables
void printMessageArchive();			// Prints all messages in the archive
void printMessage(uint8_t sender);	// Pretty prints a message
message_t *message_tx();									// Callback to Send a message
void message_rx(message_t *m, distance_measurement_t *d);	// Callback to Receive a message
void setup_message(message_t * msg, message_payload_t payload);	// Create a message with a given payload
uint8_t getMessagePayload(message_t m);	// Get payload from a given message

void messageManagementRoutine();	// Perform message management routine
void uploadWork();					// Check if i have a message to send in the queue and in affirmative case take it
void sendingAlgorithm();			// Perform a step in the sending algorithm, based on the state
void checkExpiredData();			// Check if data are still consistent


// sending algorithm functions
void pinging();
void evaluation();
void waitPing();
void rcvAvailable();
void waitAck();
void attemptsCount();
void simpleDelivery();				// Sends a message for a while (NEEDS: timeout resetted and message already in the transmitting variable)

// Message receive utils
uint8_t isNewMessage(uint8_t sender, uint8_t payload);
void notifyNewMessage(uint8_t sender);
void pushRequest(message_t mex);

// Debug
void printNeighbourList();