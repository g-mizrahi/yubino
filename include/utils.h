#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

void setup_LED(void);
void switch_LED_on(void);
void switch_LED_off(void);
void toggle_LED(void);
void setup_timer1_CTC(uint16_t, uint8_t);
void stop_timer1(void);

#endif /*UTILS_H*/