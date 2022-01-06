#pragma once

#include <Wire.h>

#define BUZZER_PIN WIO_BUZZER /* sig pin of the buzzer */

extern const int k_musicLength;
extern const int k_tempo;

struct Tune
{
    const char* note; // using preprocessor # forces type to const char*
    const int beat;
};

// used by playNote()
struct Note
{
    const char* name;
    const int tone;
};

void playNote(const char note, const int duration);

extern const Note k_noteTable[]; 
extern const Tune k_music[];
