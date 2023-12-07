#include <stdint.h>
#include <stdio.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "uart.h"

#define RX_BUFFER_SIZE 100
#define TIMER0_PRESCALER 5 // with value 5, each blocking cycle is 16ms (only values 0..5)
#define MAX_BLOCKING_CYCLES_COUNT 1

int UART__putchar(char, FILE *);
int UART__getchar(FILE *);

uint8_t rx_raw_buffer[RX_BUFFER_SIZE];
struct ring_buffer rx_buffer;
char UART_receive_buffer;

static FILE mystdout = FDEV_SETUP_STREAM(UART__putchar, NULL, _FDEV_SETUP_WRITE);
static FILE mystderr = FDEV_SETUP_STREAM(UART__putchar, NULL, _FDEV_SETUP_WRITE);
static FILE mystdin = FDEV_SETUP_STREAM(NULL, UART__getchar, _FDEV_SETUP_READ);

volatile uint8_t blocking_cycles_count;
volatile uint8_t blocking = 1; // If getchar is blocking set to 1, else set to 0

void ring_buffer__init(struct ring_buffer *rb, uint8_t *buffer, uint8_t buffer_size)
{
    rb->buffer = buffer;
    rb->maxlen = buffer_size;
    rb->head = 0;
    rb->tail = 0;
}

uint8_t
ring_buffer__pop(struct ring_buffer *rb, uint8_t *data)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t next;

        if (rb->head == rb->tail)
        { // if the head == tail, we don't have any data
            return 1;
        }

        next = rb->head + 1; // next is where head will point to after this read.
        if (next >= rb->maxlen)
        {
            next = 0;
        }

        *data = rb->buffer[rb->head]; // Read data and then move
        rb->head = next;              // tail to next offset.
    }
    return 0; // return success to indicate successful push.
}

void ring_buffer__push(struct ring_buffer *rb, uint8_t data)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        uint8_t next;

        next = rb->tail + 1; // next is where tail will point to after this write.
        if (next >= rb->maxlen)
        {
            next = 0;
        }

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            if (next == rb->head)
            { // if the tail + 1 == head, circular buffer is full
                uint8_t next_head = rb->head + 1;
                if (next_head >= rb->maxlen)
                {
                    next_head = 0;
                }
                rb->head = next_head;
            }
        }

        rb->buffer[rb->tail] = data; // Load data and then move
        rb->tail = next;             // head to next data offset.
    }
}

void set_unblocking()
{
    blocking = 0;
    TCCR0B = TIMER0_PRESCALER; // Start the timer0
}

void set_blocking()
{
    blocking = 1;
    TCCR0B = 0x0; // Stop the timer0
}

void UART__init(uint32_t baud_rate)
{
    // Set Baud Rate
    UBRR0 = (F_CPU / 16 / baud_rate) - 1;

    // enable RX, TX for UART0
    UCSR0B = _BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8 bit data format

    // Set the timer0 to use for non blocking reads
    TCCR0A = 0x0; // Overflow mode
    TCCR0B = 0x0; // Prepare the timer but don't start it unless needed to save energy
    TIMSK0 = _BV(TOIE0);

    ring_buffer__init(&rx_buffer, rx_raw_buffer, RX_BUFFER_SIZE);

    stdout = &mystdout;
    stdin = &mystdin;
    stderr = &mystderr;
    sei();
}

ISR(TIMER0_OVF_vect)
{
    blocking_cycles_count++;
}

ISR(USART_RX_vect)
{
    UART_receive_buffer = UDR0;
    if (UART_receive_buffer == '\r')
    {
        ring_buffer__push(&rx_buffer, '\n');
    }
    else
    {
        ring_buffer__push(&rx_buffer, UART_receive_buffer);
    }
}

int UART__putchar(char c, FILE *_stream)
{
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
    return 0;
}

int UART__getchar(FILE *_stream)
{
    uint8_t data;
    uint8_t read_fail = 0;
    if (blocking)
    {
        read_fail = ring_buffer__pop(&rx_buffer, &data);
        while (read_fail == 1)
        {
            read_fail = ring_buffer__pop(&rx_buffer, &data);
        }
    }
    else
    {
        // when getchar is not blocking, allow for MAX_BLOCKING_CYCLES_COUNT number of iterations of the ISR before returning
        TCNT0 = 0; // Reset timer value
        blocking_cycles_count = 0;
        read_fail = ring_buffer__pop(&rx_buffer, &data);
        while ((read_fail == 1) && (blocking_cycles_count < (MAX_BLOCKING_CYCLES_COUNT)))
        {
            read_fail = ring_buffer__pop(&rx_buffer, &data);
        }
    }
    if (read_fail == 0)
    { // Success reading data
        return (int)data;
    }
    else
    {               // Failure reading data
        return EOF; // TODO : What to return ? NULL ?
    }
}
