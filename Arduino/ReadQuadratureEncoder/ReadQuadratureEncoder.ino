#define CHA_PIN             6
#define CHB_PIN             7
#define CONTROL_PIN         8

#define SAMPLE_PERIOD       10000UL                          // us
#define MAX_COUNT           8
#define PULSES_PER_REV      5462
#define PERIODS_PER_MIN     (60000000UL/SAMPLE_PERIOD)

#define PWM_PERIOD          10

#define MEASURE_TIME        2000000                          // us
#define NUM_MEASUREMENTS    MEASURE_TIME / SAMPLE_PERIOD

#define SCALE_FACTOR        100

#define FREQUENCY_BASED

bool chA_prev, chB_prev;
//uint16_t velocities[NUM_MEASUREMENTS] = { 0 };

void setup() {

    pinMode(CHA_PIN, INPUT);
    pinMode(CHB_PIN, INPUT);
    pinMode(CONTROL_PIN, OUTPUT);

    analogWrite(CONTROL_PIN, (22*255)/100); // 1.7136ms period theoretical, 1.698ms measured
    TCCR4B = (TCCR4B & ~0x3) | 0x4;  // Set PWM prescaler to 256 for period of 8.16ms (122.5Hz)
    delay(1000);
    Serial.begin(230400);

    chA_prev = digitalRead(CHA_PIN);
    chB_prev = digitalRead(CHB_PIN);

    measure();
}

void measure() {
    uint32_t global_start_time = micros();
    char buf[64];
#ifdef FREQUENCY_BASED
    for (uint32_t i = 0; i < NUM_MEASUREMENTS; i++) {
        uint32_t start_time = micros();
        uint32_t counter = 0;

        if (micros() - global_start_time >= 1000000) {
            analogWrite(CONTROL_PIN, (25*255)/100);  // 2.041ms period theoretical, 2.018ms measured
        }

        uint32_t dt;
        while ((dt = micros() - start_time) < SAMPLE_PERIOD) {
            bool chA_cur = digitalRead(CHA_PIN);
            bool chB_cur = digitalRead(CHB_PIN);
    
            if ((chA_prev != chA_cur) || (chB_prev != chB_cur)) {
                counter++;
            }
    
            chA_prev = chA_cur;
            chB_prev = chB_cur;
        }

        uint16_t vel = (counter * PERIODS_PER_MIN) * SCALE_FACTOR / PULSES_PER_REV;
        sprintf(buf, "%ld,%d.%d\n", SAMPLE_PERIOD * i, vel / SCALE_FACTOR, vel % SCALE_FACTOR);
        Serial.print(buf);
//        velocities[i] = (counter * PERIODS_PER_MIN) * SCALE_FACTOR / PULSES_PER_REV;
    }
#else
    uint32_t i = 0;
    uint32_t intervals[NUM_MEASUREMENTS] = { 0 };
    while ((micros() - global_start_time < MEASURE_TIME) && (i < NUM_MEASUREMENTS)) {
        uint32_t start_time = micros();
        uint32_t counter = 0;

        if (micros() - global_start_time >= 1000000) {
            analogWrite(CONTROL_PIN, (25*255)/100);  // 2.041ms period
        }

        while (counter < MAX_COUNT) {
            bool chA_cur = digitalRead(CHA_PIN);
            bool chB_cur = digitalRead(CHB_PIN);
    
            if ((chA_prev != chA_cur) || (chB_prev != chB_cur)) {
                counter++;
            }
    
            chA_prev = chA_cur;
            chB_prev = chB_cur;
        }

        uint32_t dt = micros() - start_time;

        intervals[i] = dt;
        velocities[i++] = (((MAX_COUNT * 60000000) / 5462) * SCALE_FACTOR) / dt;
    }
#endif

    analogWrite(CONTROL_PIN, 0);

//    char buf[64];
//    uint32_t ms = 0;
//    for (uint32_t i = 0; i < NUM_MEASUREMENTS; i++) {   
//        uint16_t vel = velocities[i];
//        if (vel == 0) break;
//#ifdef FREQUENCY_BASED
//        sprintf(buf, "%ld,%d.%d\n", SAMPLE_PERIOD * i, vel / SCALE_FACTOR, vel % SCALE_FACTOR);
//        Serial.print(buf);
//#else
//        ms += intervals[i];
//        sprintf(buf, "%ld,%d.%d\n", ms, vel / SCALE_FACTOR, vel % SCALE_FACTOR);
//        Serial.print(buf);
//#endif
    }


    
//}

void loop() {
    
}
