#include <lilka.h>

// Перша октава
#define NOTE_C2  65
#define NOTE_D2  73
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_G2  98

// Друга октава для деяких переходів
#define NOTE_A2  110
#define NOTE_B2  123
#define NOTE_C3  131

const uint16_t melody[] = {
    NOTE_E2, NOTE_E2, NOTE_F2, NOTE_G2,
    NOTE_G2, NOTE_F2, NOTE_E2, NOTE_D2,
    NOTE_C2, NOTE_C2, NOTE_D2, NOTE_E2,
    NOTE_E2, NOTE_D2, NOTE_D2,

    NOTE_E2, NOTE_E2, NOTE_F2, NOTE_G2,
    NOTE_G2, NOTE_F2, NOTE_E2, NOTE_D2,
    NOTE_C2, NOTE_C2, NOTE_D2, NOTE_E2,
    NOTE_D2, NOTE_C2, NOTE_C2
};

const int noteDurations[] = {
    300, 300, 300, 300,
    300, 300, 300, 300,
    300, 300, 300, 300,
    450, 150, 600,

    300, 300, 300, 300,
    300, 300, 300, 300,
    300, 300, 300, 300,
    450, 150, 600
};

const int melodyLength = sizeof(melody) / sizeof(melody[0]);

void setup() {
    lilka::begin();
}

void loop() {
    for (int i = 0; i < melodyLength; i++) {
        uint16_t note = melody[i];
        int duration = noteDurations[i];

        if (note > 0) {
            lilka::buzzer.play(note);
        }
        delay(duration);
        lilka::buzzer.stop();
        delay(50); // пауза між нотами
    }

    delay(3000); // пауза перед повторенням
}
