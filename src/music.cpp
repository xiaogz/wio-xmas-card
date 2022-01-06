#include <SPI.h>

#include <music.h>

//  timeHigh = period / 2 = 1 / (2 * toneFrequency) * 10^6
//  note    frequency    period    timeHigh
//  g       392 Hz       2550       1275
//  a       440 Hz       2272       1136
//  b       493 Hz       2028       1014
//  c       523 Hz       1912        956
//  d       587 Hz       1703        851

#define N(name, tone) { #name, tone },
constexpr Note k_noteTable[] = {
    N(g, 1275)
    N(a, 1136)
    N(b, 1014)
    N(c, 956)
    N(d, 851)
};
#undef N

#define T(note, beat) { #note, beat },
// base beat length is one 16th note
#define T1(note) { #note, 1 },
// most tunes use one quarter note
#define T4(note) { #note, 4 },

constexpr Tune k_music[] = {
    T4(b)
    T4(b)
    T(b, 8)

    T4(b)
    T4(b)
    T(b, 8)

    T4(b)
    T4(d)
    T(g, 6) // dotted quarter
    T1(a) // 16th length

    T(b, 16)

    T4(c)
    T4(c)
    T(c, 6) // dotted quarter
    T1(c) // 16th length

    T4(c)
    T4(b)
    T4(b)
    T1(b) // 16th length
    T1(b) // 16th length

    T4(b)
    T4(a)
    T4(a)
    T4(b)

    T(a, 8)
    T(d, 8)
};
#undef T
#undef T4
#undef T1

constexpr int k_musicLength = sizeof(k_music) / sizeof(k_music[0]);
constexpr int k_tempo = 75;

void emitSound(const int tone, const int duration)
{
    for (long i = 0; i < duration * 1000L; i += tone * 2) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(tone);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(tone);
    }
}

void playNote(const char note, const int duration)
{
    // find sound to play
    for (uint32_t i = 0; i < sizeof(k_noteTable) / sizeof(k_noteTable[0]); i++) {
        const Note& n = k_noteTable[i];
        if (*n.name == note) {
            emitSound(n.tone, duration);
        }
    }
}

