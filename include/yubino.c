#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/atomic.h>

#include "yubino.h"
#include "utils.h"
#include "uart.h"

#define PRESCALER_500MS 4
#define OCR1A_500MS 0x7a12
#define E2START 0
#define MAX_CREDENTIAL_COUNT (E2END - E2START) / 57

volatile uint8_t button_pushed = 0;
volatile uint8_t count_half_seconds = 0;

status_t get_approval(void);
void list_credentials(void);
void reset_eeprom(void);

status_t recv_command(uint8_t command[41])
{
    set_blocking();
    fread(command, 1, 1, stdin); // Receive the command code
    switch (command[0])
    {
    case COMMAND_LIST_CREDENTIALS:
        return (STATUS_OK);

    case COMMAND_MAKE_CREDENTIAL:
        set_unblocking();
        if (fread(command + 1, 1, 20, stdin) != 20)
        {
            set_blocking();
            return (STATUS_ERR_BAD_PARAMETER);
        }
        else
        {
            set_blocking();
            return (STATUS_OK);
        }

    case COMMAND_GET_ASSERTION:
        set_unblocking();
        if (fread(command + 1, 1, 40, stdin) != 40)
        {
            set_blocking();
            return (STATUS_ERR_BAD_PARAMETER);
        }
        else
        {
            set_blocking();
            return (STATUS_OK);
        }

    case COMMAND_RESET:
        return (STATUS_OK);

    default:
        return (STATUS_ERR_BAD_PARAMETER);
    }
}

status_t exec_command(uint8_t command[41])
{
    switch (command[0])
    {
    case COMMAND_LIST_CREDENTIALS:
        list_credentials();
        return (STATUS_OK);
    case COMMAND_MAKE_CREDENTIAL:
        break;

    case COMMAND_GET_ASSERTION:
        break;

    case COMMAND_RESET:
        reset_eeprom();
        break;

    default:
        break;
    }
    return(STATUS_OK);
}

void send_error_message(status_t *status)
{
    fwrite(status, 1, 1, stdout);
}

ISR(TIMER1_COMPA_vect)
{
    count_half_seconds++;
    toggle_LED();
}

ISR(PCINT0_vect)
{
    PCMSK0 &= ~_BV(PCINT0); // Diable the interrupt on PCINT0
    button_pushed = 1;
}

status_t get_approval()
{
    button_pushed = 0;

    // Enable the interrupt on pin change for pin PCINT0 = PB0
    PCMSK0 |= _BV(PCINT0);
    PCICR = _BV(PCIE0);

    // Prepare the LED
    setup_LED();
    // Set the prescaler to 4 using the io clock
    setup_timer1_CTC(OCR1A_500MS, PRESCALER_500MS);

    NONATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        count_half_seconds = 0;
        while ((count_half_seconds < 20) && !(button_pushed))
            ;
    }

    // Disable the pin change interrupt
    PCMSK0 &= ~_BV(PCINT0);
    stop_timer1();

    if (button_pushed)
    {
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERR_APPROVAL;
    }
}

void list_credentials()
{
    uint16_t eeprom_start = E2START;
    uint8_t credential_count = eeprom_read_byte((void *)&eeprom_start);
    if (credential_count > MAX_CREDENTIAL_COUNT)
    {
        credential_count = MAX_CREDENTIAL_COUNT;
    }

    uint8_t credential_id[16];
    uint8_t app_id[20];
    uint16_t credential_id_offset = 1 + eeprom_start;
    uint16_t app_id_offset;

    putc(STATUS_OK, stdout);
    fwrite(&credential_count, 1, 1, stdout);
    for (; credential_id_offset < credential_count * 57; credential_id_offset = credential_id_offset + 57)
    {
        app_id_offset = credential_id_offset + 16;
        eeprom_read_block(credential_id, &credential_id_offset, 16);
        eeprom_read_block(app_id, &app_id_offset, 20);
        fwrite(credential_id, 1, 20, stdout);
        fwrite(app_id, 1, 16, stdout);
    }
}

void reset_eeprom()
{
}