#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdio.h>

struct ring_buffer
{
    uint8_t *buffer;
    uint8_t head;
    uint8_t tail;
    uint8_t maxlen;
};

void ring_buffer__init(struct ring_buffer *rb, uint8_t *buffer, uint8_t buffer_size);

uint8_t
ring_buffer__pop(struct ring_buffer *rb, uint8_t *data);

void ring_buffer__push(struct ring_buffer *rb, uint8_t data);

void set_blocking(void);

void set_unblocking(void);

void UART__init(uint32_t baud_rate);

int UART__putchar(char c, FILE *_stream);

int UART__getchar(FILE *_stream);

#endif /*UART_H*/