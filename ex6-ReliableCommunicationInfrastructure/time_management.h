
/******** PROTOTYPES *************************/
void setupTimeManagement();
uint8_t isInRange(uint8_t bottom, uint8_t top, clock_type_t c_type);
void resetClock(clock_type_t c_type);
uint8_t isExpired(uint8_t * timestamp);		// Determine if a given data has expired
uint8_t timeoutFires();						// Determine if timeout has elapsed