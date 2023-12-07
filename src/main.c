#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/io.h>

#include "uart.h"
#include "utils.h"
#include "yubino.h"

void setup()
{
    // set_sleep_mode(SLEEP_MODE_IDLE);
    // Init the UART connection
    UART__init(9600);
    setup_LED();
    switch_LED_off();
    sei();
}

int main()
{
    setup();

    uint8_t command[41];
    status_t status; // status code 0 means OK
    while (1)
    {
        status = STATUS_OK; // No error yet
        status = recv_command(&command);

        if (status == STATUS_OK)
        {
            // Only execute the command if there was no error on reception
            status = exec_command(&command);
        }
        else
        {
            send_error_message(&status);
            continue;
        }
    }
}