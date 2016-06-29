/*
 * Arduino library includes
 */

#include "AccelStepper.h"
#include "TaskAction.h"

/*
 * Application includes
 */

#include "speed-control.h"
#include "motion-system.h"

static const int HOMING_STEPS_PER_S = mm_units_to_steps(150, MM_PER_STEP);

static const int MAXIMUM_DISTANCE_MM = 2800;
static const int MAXIMUM_DISTANCE_STEPS = mm_units_to_steps(MAXIMUM_DISTANCE_MM, MM_PER_STEP);

static const float MAXIMUM_SPEED_MM_PER_S = 1000;
static const float MAXIMUM_SPEED_STEPS_PER_S = mm_units_to_steps(MAXIMUM_SPEED_MM_PER_S, MM_PER_STEP);

/* A4988 Pins */
static const int A4988A_DIR = 14;
static const int A4988A_STEP = 15;
static const bool FORWARD_DIRECTION_HIGH = false;

/* Other Pins */
static const int HOME_PIN = 6;

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

static void print_speed_task_fn()
{
	float speed = speed_get_mm_per_s();

	Serial.print("Speed ");
	Serial.print(speed);
	Serial.print(" mm/s, ");
	Serial.print(mm_units_to_steps(speed, MM_PER_STEP));
	Serial.println(" steps/s");
}
static TaskAction s_print_speed_task(print_speed_task_fn, 1000, INFINITE_TICKS);

static void io_setup()
{
	pinMode(A4988A_STEP, OUTPUT);
	pinMode(A4988A_DIR, OUTPUT);

	pinMode(HOME_PIN, INPUT_PULLUP);

	speed_setup_io();
}

static inline bool motor_at_home()
{
	static int debounce = 0;
	debounce += digitalRead(HOME_PIN) == LOW ? 1 : -1;

	if (debounce > 100) {debounce = 100;}
	if (debounce < 0) {debounce = 0;}

	return debounce == 100;
}

static void move_clear_of_microswitch(AccelStepper& stepper)
{
	stepper.setCurrentPosition(0);
	stepper.moveTo(50);
	stepper.setSpeed(HOMING_STEPS_PER_S);
	while(stepper.distanceToGo())
	{
		stepper.runSpeedToPosition();
	}
}

static void go_home(AccelStepper& stepper)
{
	stepper.setCurrentPosition(0);
	stepper.setMaxSpeed(HOMING_STEPS_PER_S);
	stepper.setSpeed(-HOMING_STEPS_PER_S);

	while (!motor_at_home() && (-stepper.currentPosition() < MAXIMUM_DISTANCE_STEPS))
	{
		stepper.runSpeed();
	}

	if (!motor_at_home())
	{
		Serial.print("Could not find home within ");
		Serial.print(MAXIMUM_DISTANCE_MM);
		Serial.println("mm travel");

		while(true) {};
	}

	delay(100);

	move_clear_of_microswitch(stepper);
}

static void setup_for_run(AccelStepper& stepper)
{
	stepper.setCurrentPosition(0);
	stepper.setAcceleration( speed_get_motor_accel(true) );
	stepper.setMaxSpeed( speed_get_maximum_speed() );
}

static void move_distance(AccelStepper& stepper, long steps)
{
	float speed = 0.0;

	stepper.move( steps );
	while(stepper.run()) {
		speed_update();
		speed = mm_units_to_steps(speed_get_mm_per_s(), MM_PER_STEP);
		stepper.setMaxSpeed(speed);
		s_print_speed_task.tick();
	}
	delay(10);
}

void setup()
{

	float speed_mm_per_s = speed_get_mm_per_s();

	delay(3000);

	Serial.begin(115200);

	Serial.println("Linear Actuator v1");
	Serial.println();
	Serial.print("Accel, mm/s2: "); Serial.println( speed_get_motor_accel(false) );
	Serial.print("Accel, steps/s2: "); Serial.println( speed_get_motor_accel(true) );

	io_setup();
	
	Serial.println("Homing...");

	go_home(s_stepper);

	Serial.println("Running...");

	Serial.print("Speed, mm/s: "); Serial.println(speed_mm_per_s);
	Serial.print("Speed, steps/s: "); Serial.println(mm_units_to_steps(speed_mm_per_s, MM_PER_STEP)); 

	setup_for_run(s_stepper);

}

void loop()
{
	Serial.print("Moving forward "); Serial.print(MAXIMUM_DISTANCE_MM); Serial.print("mm ("); Serial.print(MAXIMUM_DISTANCE_STEPS); Serial.println(") steps");
	move_distance(s_stepper, MAXIMUM_DISTANCE_STEPS);

	Serial.print("Moving backward "); Serial.print(MAXIMUM_DISTANCE_MM); Serial.print("mm ("); Serial.print(MAXIMUM_DISTANCE_STEPS); Serial.println(") steps");
	move_distance(s_stepper, -MAXIMUM_DISTANCE_STEPS);
}
