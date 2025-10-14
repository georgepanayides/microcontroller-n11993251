#include <stdint.h>

/** EX: 7.5

To allow the variables "count_64" and "is_counting" to be shared between
compilation units (i.e., src/timer.c and src/buttons.c), we must declare
them as external variables.

TASK: Declare "count_64" and "is_counting" as external variables.

NOTE: An external declaration must not provide an initialisation.
*/

extern volatile uint16_t count_64;
extern volatile uint8_t is_counting;


/** CODE: Write your code for Ex 7.5 above this line. */
