#pragma once
// All possible tone states
typedef enum tone_states {
	tone_nominal,
	tone_alert,
	tone_evac
} tone_states;


// All possible sector states
typedef enum sector_states {
	sensor_nominal,
	alert_sense,
	alert_trigger,
	alert_isolate,
	alert_evac
} sector_states;


// All possible reset states
typedef enum reset_states {
	reset_nominal,
	reset_1,
	reset_2
} reset_states;

// Struct for referencing the reset state
typedef struct reset_state {
	enum reset_states state;
} reset_state;

// Struct for referencing the sector state
typedef struct sector_state {
	enum sector_states state;
} sector_state;

// Struct for referencing the tone state
typedef struct tone_state {
	enum tone_states state;
} tone_state;

// State used to hold the whole state of the alarm
typedef struct major_state {
	tone_state * tone_state;
	reset_state * reset_state;
	sector_state * sector_1;
	sector_state * sector_2;
	sector_state * sector_3;
} major_state;
