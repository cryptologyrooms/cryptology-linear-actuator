/*
 * Arduino library includes
 */

#include "AccelStepper.h"

/*
 * Application includes
 */

#include "speed_control.h"

/* Motion system functions */
static float mm_units_to_steps(float mm_units, float mm_per_step)
{
	return mm_units / mm_per_step;
}

/* Motion system parameters */
static const float PULLEY_DIAMETER_MM = 19.09;
static const float STEPS_PER_REV = 200;
static const float MM_PER_STEP = M_PI * PULLEY_DIAMETER_MM/STEPS_PER_REV;

static const float MAXIMUM_SPEED_MM_PER_S = 100;
static const float MAXIMUM_SPEED_STEPS_PER_S = mm_units_to_steps(MAXIMUM_SPEED_MM_PER_S, MM_PER_STEP);

static const int ACCELERATION_MM_PER_S2 = 1000;
static const int ACCELERATION_STEPS_PER_S2 = mm_units_to_steps(ACCELERATION_MM_PER_S2, MM_PER_STEP);

static const int HOMING_STEPS_PER_S = mm_units_to_steps(100, MM_PER_STEP);

static const int MAXIMUM_DISTANCE_MM = 6000;
static const int MAXIMUM_DISTANCE_STEPS = mm_units_to_steps(MAXIMUM_DISTANCE_MM, MM_PER_STEP);

/* A4988 Pins */
static const int A4988A_DIR = 2;
static const int A4988A_STEP = 3;
static const bool FORWARD_DIRECTION_HIGH = false;

/* Other Pins */
static const int HOME_PIN = 12;

#ifdef USE_FAST_IO
#define STEP_PORT PORTD
#define STEP_PIN 2
#endif

/* Local Variables */
static float s_speed_mm_per_s = 0;
static bool s_direction_fwd = true;

/* AccelStepper step callbacks */

static void step()
{
	#ifndef USE_FAST_IO
	digitalWrite(A4988A_STEP, HIGH); 
    delayMicroseconds(1); 
    digitalWrite(A4988A_STEP, LOW);
    #else
    PORTD |= _BV(STEP_PIN);
    _delay_us(1);
    PORTD &= ~_BV(STEP_PIN);
    #endif
}

static void set_direction(bool forward)
{
	if (forward != s_direction_fwd)
	{
		digitalWrite(A4988A_DIR, FORWARD_DIRECTION_HIGH == forward ? HIGH : LOW);
		s_direction_fwd = forward;
	}
}

static void step_fwd()
{
	set_direction(true);
	step();
}

static void step_bck()
{
	set_direction(false);
	step();
}

static AccelStepper s_stepper(step_fwd, step_bck);


/**************
 * TEST PARAMETERS
 **************/

static const int TEST_DISTANCE_MM = 2500;

/**************
 * END TEST PARAMETERS
 **************/

static void io_setup()
{
	pinMode(A4988A_STEP, OUTPUT);
	pinMode(A4988A_DIR, OUTPUT);

	pinMode(HOME_PIN, INPUT_PULLUP);
}

static void go_home(AccelStepper& stepper)
{
	stepper.setCurrentPosition(0);
	stepper.setSpeed(-HOMING_STEPS_PER_S);

	while ((digitalRead(HOME_PIN) == HIGH) && (-stepper.currentPosition() < MAXIMUM_DISTANCE_STEPS))
	{
		stepper.runSpeed();
	}

	if (digitalRead(HOME_PIN) == HIGH)
	{
		Serial.print("Could not find home within ");
		Serial.print(MAXIMUM_DISTANCE_MM);
		Serial.print("mm travel");

		while(true) {};
	}
}

static void setup_for_run(AccelStepper& stepper, float speed_steps_per_s)
{
	stepper.setCurrentPosition(0);
	stepper.setAcceleration( ACCELERATION_STEPS_PER_S2 );
	stepper.setMaxSpeed( speed_steps_per_s );
}

void setup()
{

	float speed_mm_per_s = get_speed_mm_per_s();

	Serial.begin(115200);

	Serial.println("Linear Actuator v1");
	Serial.println();
	Serial.print("Accel, mm/s2: "); Serial.println(ACCELERATION_MM_PER_S2);
	Serial.print("Accel, steps/s2: "); Serial.println(ACCELERATION_STEPS_PER_S2);

	io_setup();
	
	Serial.println("Homing...");

	go_home(s_stepper);

	Serial.println("Running...");

	Serial.print("Speed, mm/s: "); Serial.println(speed_mm_per_s);
	Serial.print("Speed, steps/s: "); Serial.println(mm_units_to_steps(speed_mm_per_s, MM_PER_STEP)); 

	setup_for_run(s_stepper, mm_units_to_steps(speed_mm_per_s, MM_PER_STEP));
	
}

void loop()
{
	float steps = mm_units_to_steps(TEST_DISTANCE_MM, MM_PER_STEP);
	delay(10);

	Serial.print("Moving forward "); Serial.print(TEST_DISTANCE_MM); Serial.print("mm ("); Serial.print(steps); Serial.println(") steps");
	s_stepper.move( steps );
	while(s_stepper.run()) {}

	delay(10);

	Serial.print("Moving backward "); Serial.print(TEST_DISTANCE_MM); Serial.print("mm ("); Serial.print(steps); Serial.println(") steps");
	s_stepper.move( -steps );
	while(s_stepper.run()) {}
}