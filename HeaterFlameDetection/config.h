#ifndef CONFIG_H
#define CONFIG_H

// --- Pin assignments ---
#define WGT_SENSOR_PIN     A0  // PT1000 + 10k divider (PT1000 -> VCC, 10k -> GND, mid -> A0)
#define RELAY_PIN          2   // Active LOW relay — bridges the boiler's waste-gas thermostat
#define GREEN_LED_PIN      3   // Flame indicator (blink = igniting, solid = burning)
#define RED_LED_PIN        4   // Failure indicator (ignition timeout)
#define BUZZER_PIN         5   // Failure alarm (active buzzer)

// --- Sampling intervals (ms) ---
#define FAST_INTERVAL      50      // PT1000 read frequency
#define SLOW_INTERVAL      1000    // Slow average + gradient evaluation

// --- Moving average smoothing factors ---
#define FAST_AVG_ALPHA     0.3
#define SLOW_AVG_ALPHA     0.035

// --- Flame detection thresholds ---
#define WGT_ON             85      // °C — boiler considered burning above this
#define WGT_OFF            75      // °C — boiler considered cold below this
#define GRADIENT_MIN       3.0     // °C/min — minimum rise rate to count an ignition sample
#define GRADIENT_MAX       15.0    // °C/min — maximum rise rate (filters out noise spikes)
#define GRADIENT_TRY_COUNT 7       // Valid samples required before engaging relay

// --- Timeouts (ms) ---
#define IGNITION_TIMEOUT     3600000UL  // 1 h — from relay engage to confirmed flame (WGT > WGT_ON)
#define MINIMUM_BURN_TIME     900000UL  // 15 min — minimum burn time before returning to IDLE
#define BUZZER_DURATION       120000UL  // 2 min — alarm length after ignition failure
#define FAILED_RESET_TIME     300000UL  // 5 min — auto-reset from FAILED once temperature drops

// --- LED blink ---
#define BLINK_DURATION        200       // ms — green flash length per valid gradient sample

#endif
