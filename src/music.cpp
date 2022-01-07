#include <music.h>

#include <SPI.h>
#include <Wire.h>

#define BUZZER_PIN WIO_BUZZER /* sig pin of the buzzer */

//  timeHigh = period / 2 = 1 / (2 * frequency) * 10^6
//  note    frequency    period    timeHigh
//  g       392 Hz       2550       1275
//  a       440 Hz       2272       1136
//  b       493 Hz       2028       1014
//  c       523 Hz       1912        956
//  d       587 Hz       1703        851

// used by playNote()
struct Note
{
    const char* name;
    const int timeHigh;
};

#define N(name, timeHigh) { #name, timeHigh },
constexpr Note k_noteTable[] = {
    N(g, 1275)
    N(a, 1136)
    N(b, 1014)
    N(c, 956)
    N(d, 851)
};
#undef N

struct Tune
{
    const char* note; // using preprocessor # forces type to const char*
    const int beat;
};

#define T(note, beat) { #note, beat },
// base beat length is one 16th note
#define T1(note) { #note, 1 },
// most tunes use one quarter note
#define T4(note) { #note, 4 },

// jingle bells
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
constexpr int k_ISRDebounceMs = 200;

static bool g_isPlaying = true;
static int g_playerPosition = 0;

void emitSound(const int timeHigh, const int duration)
{
    for (long i = 0; i < duration * 1000L; i += timeHigh * 2) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(timeHigh);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(timeHigh);
    }
}

void playNote(const char note, const int duration)
{
    // find sound to play
    for (uint32_t i = 0; i < sizeof(k_noteTable) / sizeof(k_noteTable[0]); i++) {
        const Note& n = k_noteTable[i];
        if (*n.name == note) {
            emitSound(n.timeHigh, duration);
        }
    }
}

// ISRs signature: no input, void return
void ISRPlayPause()
{
    static unsigned long last_interrupt_time = 0;
    const unsigned long interrupt_time = millis();
    // If interrupts come faster than k_ISRDebounceMs, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > k_ISRDebounceMs)
    {
        g_isPlaying = !g_isPlaying;
    }
    last_interrupt_time = interrupt_time;
}

void playMusic()
{
    if (!g_isPlaying) return;

    int& i = g_playerPosition;

    for(; i < k_musicLength; i++) {
        if (!g_isPlaying) break;

        const Tune& t = k_music[i];
        if(*t.note == ' ') {
            delay(t.beat * k_tempo);
        }
        else {
            playNote(*t.note, t.beat * k_tempo);
        }

        delay(k_tempo / 2); // delay between notes
    }

    if (i >= k_musicLength) i = 0;

    delay(k_tempo * 3);
}

void setupMusic()
{
    // activate sound output
    pinMode(BUZZER_PIN, OUTPUT);

    // Button map when looking at 3 buttons with screen facing up
    // button 1 - a innermost
    // button 2 - b between 1a and 3c
    // button 3 - c outermost edge

    // enable middle button to pause music
    Serial.begin(115200);
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), ISRPlayPause, LOW);
}

