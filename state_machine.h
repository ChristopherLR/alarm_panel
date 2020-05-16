#pragma once

typedef enum tone_states {
	tone_nominal,
	tone_alert,
	tone_evac
} tone_states;

typedef enum sector_states {
	sensor_nominal,
	alert_sense,
	alert_trigger,
	alert_isolate,
	alert_evac
} sector_states;

typedef enum reset_states {
	reset_nominal,
	reset_1,
	reset_2
} reset_states;

typedef struct reset_state {
	enum reset_states state;
} reset_state;

typedef struct sector_state {
	enum sector_states state;
} sector_state;

typedef struct tone_state {
	enum tone_states state;
} tone_state;

typedef struct major_state {
	tone_state * tone_state;
	reset_state * reset_state;
	sector_state * sector_1;
	sector_state * sector_2;
	sector_state * sector_3;
} major_state;
