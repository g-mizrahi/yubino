#include <avr/io.h>
#include <util/atomic.h>

void setup_LED()
{
    DDRB |= _BV(DDB5);
}

void switch_LED_on()
{
    PORTB |= _BV(PORTB5);
}

void switch_LED_off()
{
    PORTB &= ~_BV(PORTB5);
}

void toggle_LED()
{
    PORTB ^= _BV(PORTB5);
}

void setup_timer1_CTC(uint16_t ocr1a, uint8_t prescaler)
{
    TCCR1B = prescaler | _BV(WGM12); // set the prescaler on the io clock in CTC mode

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        OCR1A = ocr1a; // set the TOP value
        TCCR1A = 0;    // reset the counter
    }

    TIMSK1 = _BV(OCIE1A); // enable the interrupts
}

void stop_timer1()
{
    TCCR1B = 0x00;
}