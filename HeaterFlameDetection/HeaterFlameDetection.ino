// HeaterFlameDetection
// Standalone flame watchdog for solid-fuel boilers.
//
// Concept:
//   1. PT1000 in the flue measures waste gas temperature.
//   2. While the boiler is cold, the firmware watches for a sustained
//      temperature rise (gradient between GRADIENT_MIN and GRADIENT_MAX).
//   3. After GRADIENT_TRY_COUNT valid samples it engages the relay,
//      bridging the boiler's mechanical waste-gas thermostat. From now on
//      this device holds the boiler running.
//   4. If a real flame is confirmed (WGT > WGT_ON) within IGNITION_TIMEOUT,
//      the green LED stays solid.
//   5. If the timeout expires without a real flame, the relay releases,
//      red LED + buzzer signal an ignition failure.
//   6. When the boiler eventually cools down (WGT < WGT_OFF), state
//      returns to IDLE.
//
// LED behaviour:
//   - off               — idle, nothing detected
//   - green blinking    — gradient detected, boiler is starting up
//   - green solid       — flame confirmed, boiler is burning
//   - red + buzzer      — ignition failed (timeout)
//   - red solid         — same, after the buzzer auto-silences

#include "config.h"
#include <math.h>

enum State {
  STATE_IDLE,      // waiting, nothing happening
  STATE_COUNTING,  // accumulating valid gradient samples
  STATE_BURNING,   // relay engaged, flame detected (or about to be)
  STATE_FAILED     // ignition timeout
};

static State state = STATE_IDLE;

// Temperature averages (double — same as the parent project)
static double fastAvg = 0;
static double slowAvg = 0;

// Gradient tracking
static int   previousSlowTemp     = 0;
static unsigned long lastSlowEvalMillis = 0;
static int   gradientCount        = 0;

// Loop timing
static unsigned long lastFastReadMillis = 0;
static unsigned long lastSlowReadMillis = 0;

// State timing
static unsigned long burningStartMillis = 0;
static unsigned long failedStartMillis  = 0;

// Green blink off-timer
static unsigned long greenBlinkOffMillis = 0;

// ---------------------------------------------------------------------------

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);   // released (active LOW)

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Serial.begin(9600);
  Serial.println(F("HeaterFlameDetection ready"));
}

// ---------------------------------------------------------------------------
// PT1000 read — same Callendar-Van Dusen approximation as the parent project.
// Returns temperature in °C or -1 if reading is invalid.
int readWasteGasTemperature() {
  uint32_t raw = analogRead(WGT_SENSOR_PIN);
  raw = 1024 - raw;
  if (raw == 0 || raw >= 1024) return -1;

  double R1 = (raw * 10000.0) / (1024.0 - raw);
  double inside = (-0.00232 * R1) + 17.59246;
  if (inside < 0) return -1;

  int T = (int)((sqrt(inside) - 3.908) / 0.00116) * (-1);
  if (T <= 0 || T >= 450) return -1;
  return T;
}

void updateFastAverage() {
  int T = readWasteGasTemperature();
  if (T < 0) return;
  if (fastAvg == 0) fastAvg = T;
  else fastAvg = FAST_AVG_ALPHA * T + (1 - FAST_AVG_ALPHA) * fastAvg;
}

// ---------------------------------------------------------------------------
// Slow average + gradient evaluation. Mirrors ComputeSlowWasteGasTemperature
// from the parent project but only acts on flame detection.
void updateSlowAverageAndGradient() {
  if (slowAvg == 0) {
    slowAvg = fastAvg;
    previousSlowTemp = (int)slowAvg;
    lastSlowEvalMillis = millis();
    return;
  }

  int prev = (int)slowAvg;
  slowAvg = SLOW_AVG_ALPHA * fastAvg + (1 - SLOW_AVG_ALPHA) * slowAvg;

  unsigned long now = millis();
  float minutesElapsed = (now - lastSlowEvalMillis) / 60000.0;
  if (minutesElapsed <= 0) return;
  if ((int)slowAvg == prev) return;

  float gradient = ((int)slowAvg - prev) / minutesElapsed;
  lastSlowEvalMillis = now;

  // Gradient detection only matters while the boiler is still cold and we
  // haven't already committed to a burn cycle.
  if (state != STATE_IDLE && state != STATE_COUNTING) return;

  bool isCold = slowAvg < WGT_OFF;

  if (gradient > GRADIENT_MIN && gradient < GRADIENT_MAX && isCold) {
    gradientCount++;
    flashGreen();
    if (state == STATE_IDLE) state = STATE_COUNTING;

    if (gradientCount > GRADIENT_TRY_COUNT) {
      enterBurning(now);
    }
  } else if (gradient <= GRADIENT_MIN && gradientCount > 0) {
    gradientCount--;
    if (gradientCount == 0 && state == STATE_COUNTING) {
      state = STATE_IDLE;
    }
  }
}

// ---------------------------------------------------------------------------
// State transitions

void enterBurning(unsigned long now) {
  state = STATE_BURNING;
  burningStartMillis = now;
  gradientCount = 0;
  digitalWrite(RELAY_PIN, LOW);          // engage — bridge the thermostat
  digitalWrite(GREEN_LED_PIN, HIGH);     // solid green
  greenBlinkOffMillis = 0;
  Serial.println(F("STATE: BURNING"));
}

void enterIdle() {
  state = STATE_IDLE;
  gradientCount = 0;
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  greenBlinkOffMillis = 0;
  Serial.println(F("STATE: IDLE"));
}

void enterFailed(unsigned long now) {
  state = STATE_FAILED;
  failedStartMillis = now;
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  Serial.println(F("STATE: FAILED"));
}

// ---------------------------------------------------------------------------
// Per-state housekeeping

void flashGreen() {
  digitalWrite(GREEN_LED_PIN, HIGH);
  greenBlinkOffMillis = millis() + BLINK_DURATION;
}

void updateBlink() {
  if (state != STATE_COUNTING) return;
  if (greenBlinkOffMillis != 0 && millis() >= greenBlinkOffMillis) {
    digitalWrite(GREEN_LED_PIN, LOW);
    greenBlinkOffMillis = 0;
  }
}

void checkBurning() {
  if (state != STATE_BURNING) return;
  unsigned long now = millis();
  unsigned long elapsed = now - burningStartMillis;

  // Boiler dropped back to cold — burn cycle finished, release everything.
  if (slowAvg < WGT_OFF && elapsed > MINIMUM_BURN_TIME) {
    enterIdle();
    return;
  }

  // Gradient pulled the trigger but flame was never confirmed in time.
  if (slowAvg < WGT_ON && elapsed > IGNITION_TIMEOUT) {
    enterFailed(now);
  }
}

void checkFailed() {
  if (state != STATE_FAILED) return;
  unsigned long now = millis();
  unsigned long elapsed = now - failedStartMillis;

  // Silence the buzzer after a few minutes; keep red LED on.
  if (elapsed > BUZZER_DURATION) {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Auto-reset once the operator has cleared the situation and the boiler
  // is cold again.
  if (slowAvg < WGT_OFF && elapsed > FAILED_RESET_TIME) {
    enterIdle();
  }
}

// ---------------------------------------------------------------------------

void loop() {
  unsigned long now = millis();

  if (now - lastFastReadMillis >= FAST_INTERVAL) {
    lastFastReadMillis = now;
    updateFastAverage();
  }

  if (now - lastSlowReadMillis >= SLOW_INTERVAL) {
    lastSlowReadMillis = now;
    updateSlowAverageAndGradient();
  }

  updateBlink();
  checkBurning();
  checkFailed();
}
