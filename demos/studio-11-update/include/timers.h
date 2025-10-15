#include <stdint.h>

extern volatile uint16_t elapsed_time;
extern volatile uint16_t playback_delay;
extern volatile uint16_t new_playback_delay;

void prepare_delay(void);

extern volatile uint8_t pb_debounced_state;
