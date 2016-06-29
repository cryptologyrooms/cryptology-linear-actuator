/*
 * Arduino library includes
 */

#include "Arduino.h"


/*
 * Application includes
 */

#include "speed-control.h"
#include "motion-system.h"

static const int NUMBER_OF_SPEEDS = 6;

static const int PINS[NUMBER_OF_SPEEDS] = {7, 8, 9, 10, 11, 12};
static const int MAX_COUNT = 1000;

static const float MAXIMUM_SPEED_MM_PER_S = 500;
static const float MAXIMUM_SPEED_STEPS_PER_S = mm_units_to_steps(MAXIMUM_SPEED_MM_PER_S, MM_PER_STEP);

static const int ACCELERATION_MM_PER_S2 = 800;
static const int ACCELERATION_STEPS_PER_S2 = mm_units_to_steps(ACCELERATION_MM_PER_S2, MM_PER_STEP);

static float SPEEDS_MM_PER_S[NUMBER_OF_SPEEDS] = {
	100, 150, 200, 250, 300, MAXIMUM_SPEED_MM_PER_S};
	
static float s_last_good_speed = MAXIMUM_SPEED_MM_PER_S;

static int pin_states[NUMBER_OF_SPEEDS] = {0,0,0,0,0,0};

/*
 * Public module functions
 */

float mm_units_to_steps(float mm_units, float mm_per_step)
{
	return mm_units / mm_per_step;
}

int speed_get_motor_accel(bool steps_per_s)
{
	return steps_per_s ? ACCELERATION_STEPS_PER_S2 : ACCELERATION_MM_PER_S2;
}

int speed_get_maximum_speed()
{
	return MAXIMUM_SPEED_STEPS_PER_S;
}

void speed_print_pin_states()
{
	Serial.println("Pin states: ");
	for (int i = 0; i < NUMBER_OF_SPEEDS; i++)
	{
		Serial.print(i);
		Serial.print(": ");
		Serial.println(pin_states[i]);
	}
}

void speed_setup_io()
{
	for (int i = 0; i < NUMBER_OF_SPEEDS; i++)
	{
		pinMode(PINS[i], INPUT_PULLUP);
	}

	Serial.print("Available speeds, mm/s: ");
	for (int i = 0; i < NUMBER_OF_SPEEDS; i++)
	{
		Serial.print(SPEEDS_MM_PER_S[i]);
		if (i < NUMBER_OF_SPEEDS-1) {Serial.print(", ");}
	}
	Serial.println();
}

float speed_update()
{
	for (int i = 0; i < NUMBER_OF_SPEEDS; i++)
	{
		pin_states[i] += digitalRead(PINS[i]) == LOW ? 1 : -1;
		if (pin_states[i] < 0) { pin_states[i] = 0; }
		if (pin_states[i] > MAX_COUNT) { pin_states[i] = MAX_COUNT; }

		if (pin_states[i] == MAX_COUNT)
		{
			s_last_good_speed = SPEEDS_MM_PER_S[i];
		}
	}

	return s_last_good_speed;
}

float speed_get_mm_per_s()
{
	return s_last_good_speed;
}