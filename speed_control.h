#ifndef _SPEED_CONTROL_H_
#define _SPEED_CONTROL_H_

void speed_setup_io();
void speed_print_pin_states();

float speed_update();
float speed_get_mm_per_s();

#endif