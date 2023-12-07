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

volatile uint8_t button_pushed;
volatile uint8_t count_half_seconds;

status_t get_approval(void);
status_t list_credentials(void);
status_t reset_eeprom(void);
status_t make_credential(void);

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
        return (STATUS_ERR_COMMAND_UNKOWN);
    }
}

status_t exec_command(uint8_t command[41])
{
    status_t status;
    switch (command[0])
    {
    case COMMAND_LIST_CREDENTIALS:
        status = list_credentials();
        return (status);

    case COMMAND_MAKE_CREDENTIAL:
        break;

    case COMMAND_GET_ASSERTION:
        break;

    case COMMAND_RESET:
        status = get_approval();
        if (status == STATUS_OK)
        {
            status = reset_eeprom();
        }
        return (status);

    default:
        break;
    }
    return (STATUS_OK);
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
    switch_LED_off();

    if (button_pushed)
    {
        return STATUS_OK;
    }
    else
    {
        return STATUS_ERR_APPROVAL;
    }
}

status_t list_credentials()
{
    uint8_t credential_total = eeprom_read_byte((uint8_t *)E2START);

    if (credential_total > MAX_CREDENTIAL_COUNT)
    {
        credential_total = MAX_CREDENTIAL_COUNT;
    }

    uint8_t credential_id[16];
    uint8_t app_id[20];

    putc(STATUS_OK, stdout);
    putc(credential_total, stdout);
    for (uint16_t credential_address = 1 + E2START; credential_address < credential_total * 57; credential_address = credential_address + 57)
    {
        // Read the credential id
        eeprom_read_block(credential_id, (void *)credential_address, 16);

        eeprom_read_block(app_id, (void *)credential_address + 16, 20);

        // print the data
        fwrite(credential_id, 1, 16, stdout);
        fwrite(app_id, 1, 20, stdout);
    }
    return (STATUS_OK);
}

status_t reset_eeprom()
{
    uint8_t credential_total = eeprom_read_byte((uint8_t *)E2START);
    for (uint16_t eeprom_address = E2START; eeprom_address <= E2START + 57 * credential_total; eeprom_address++)
    {
        eeprom_update_byte((uint8_t *)eeprom_address, 0xff);
    }
    putc(STATUS_OK, stdout);
    return (STATUS_OK);
}