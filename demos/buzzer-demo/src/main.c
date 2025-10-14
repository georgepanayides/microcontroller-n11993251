#include <avr/io.h>
#include <stdio.h>

#include "qutyserial.h"

// Prototype for the function you will implement in extension05.c
void init(void);

int main(void)
{
    serial_init();

    // Call init() and check that it returns
    printf("Ex E5.0: Calling init()...");
    init();
    printf("DONE.\n");

    while (1)
        ; // Loop indefinitely
}
