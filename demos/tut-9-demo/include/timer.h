#include <stdint.h>

// 1 = not pressed (pullup), 0 = pressed
extern volatile uint8_t pb_debounced_state;

void buttons_init(void);
void timer_init(void);
