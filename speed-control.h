#ifndef _SPEED_CONTROL_H_
#define _SPEED_CONTROL_H_

float mm_units_to_steps(float mm_units, float mm_per_step);

void speed_setup_io();
void speed_print_pin_states();

int speed_get_motor_accel(bool steps_per_s);
int speed_get_maximum_speed();

float speed_update();
float speed_get_mm_per_s();

#endif