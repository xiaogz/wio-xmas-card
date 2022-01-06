#include <SPI.h> // required to be with setup() & loop()

#include <display.h>
#include <music.h>

void setup()
{
    setupDisplay();
    setupMusic();
}

void loop()
{
    playMusic();
}

