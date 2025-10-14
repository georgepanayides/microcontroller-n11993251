#include "timer.h"

#include <avr/io.h>
#include <stdint.h>

uint16_t playback_delay = 100;

void update_playback_delay(void)
{
    /** EX: 11.0.2

    To control the difficulty of this game, the player can use the
    potentiometer to adjust the duration of the outputs produced by the
    program.

    TASK: Use ADC0 to update the "playback_delay" variable based on the
    position of the potentiometer. This value should be a linear
    interpolation of the position of the potentiometer, ranging between
    100 ms and 500 ms.

    In "initialisation.c:adc_init", free-running mode has been disabled
    to improve program efficiency and reduce sampling inconsistency.
    Due to this, we must manually start conversions when this function
    is called and wait for them to complete.

    Use the following steps to achieve this:

    1. Start a conversion using the START group configuration in the
       COMMAND register.
    2. Wait for a conversion to complete before updating the playback
       delay. See the Result Ready flag in the INTFLAGS register.
    3. Upon reading the conversion result from the RESULT register,
       clear the appropriate flag in the INTFLAGS register.
    */

    /** CODE: Write your code for Ex 11.0.2 within this function. */

   #ifdef ADC_STCONV_bm
   ADC0.COMMAND = ADC_STCONV_bm;
   #else
   ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;
   #endif

   // wait for result ready flag (RESRDY)
   while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0) {
   }

   // read the result and clear the flag
   uint16_t result = ADC0.RESULT; // read conversion result
   ADC0.INTFLAGS = ADC_RESRDY_bm; // write1toclear

   // playback_delay calc
   playback_delay = (uint16_t)((((uint16_t)(500 - 100) * result) >> 8) + 100);

}