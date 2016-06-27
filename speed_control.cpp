/*
 * Arduino library includes
 */

#include "Arduino.h"


/*
 * Application includes
 */

#include "speed_control.h"

static const int NUMBER_OF_SPEEDS = 6;

static const int PINS[NUMBER_OF_SPEEDS] = {7, 8, 9, 10, 11, 12};
static const int MAX_COUNT = 1000;

static const int TEST_SPEED_MM_PER_S = 1500;

static float SPEEDS_MM_PER_S[NUMBER_OF_SPEEDS] = 
		{TEST_SPEED_MM_PER_S, TEST_SPEED_MM_PER_S, TEST_SPEED_MM_PER_S, TEST_SPEED_MM_PER_S, TEST_SPEED_MM_PER_S, TEST_SPEED_MM_PER_S};

static float s_last_good_speed = TEST_SPEED_MM_PER_S;

/*
 * Private module functions
 */

static int get_speed_selection()
{
	

}

/*
 * Public module functions
 */

float get_speed_mm_per_s()
{

	static int pin_states[NUMBER_OF_SPEEDS] = {0,0,0,0,0,0};

	for (int i = 0; i < NUMBER_OF_SPEEDS; i++)
	{
		pin_states[i] += digitalRead(PINS[i]) == LOW ? 1 : -1;
		
		if (pin_states[i] < 0) { pin_states[i] = 0; }
		if (pin_states[i] > MAX_COUNT) { pin_states[i] = MAX_COUNT; }

		if (pin_states[i] == 0)
		{
			s_last_good_speed = SPEEDS_MM_PER_S[i];
		}
	}

	return s_last_good_speed;
}