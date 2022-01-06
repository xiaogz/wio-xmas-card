#include <Wire.h>
#include <SPI.h>
#include <TFT_eSPI.h>

// Button map when looking at 3 buttons with screen facing up
// button 1 - a innermost
// button 2 - b between 1a and 3c
// button 3 - c outermost edge

// TODO: set UTC time during setup or else wifi credentials look "expired" and won't connect
// TODO: connect to Azure IoT Hub
// TODO: learn how to setup Azure IoT Hub (security, connection infrastructure)
// if upload fails due to "cannot find device on COM#" you need to restart vscode

#define BUZZER_PIN WIO_BUZZER /* sig pin of the buzzer */

// jingle bells
struct Tune
{
    const char* note; // using preprocessor # forces type to const char*
    const int beat;
};

#define T(note, beat) { #note, beat },
// base beat length is one 16th note
#define T1(note) { #note, 1 },
// most uses one quarter note
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

constexpr int k_musicLength = sizeof(k_music) / sizeof(k_music[0]);
constexpr int k_tempo = 75;

constexpr int k_ISRDebounceMs = 200;

// used by playNote()
// TODO: implement hashtable
struct Note
{
    const char* name;
    const int tone;
};

//  timeHigh = period / 2 = 1 / (2 * toneFrequency) * 10^6
//  note    frequency    period    timeHigh
//  c       261 Hz       3830       1915
//  d       294 Hz       3400       1700
//  e       329 Hz       3038       1519
//  f       349 Hz       2864       1432
//  g       392 Hz       2550       1275
//  a       440 Hz       2272       1136
//  b       493 Hz       2028       1014
//  C       523 Hz       1912        956
//  D       587 Hz       1703        851

#define N(name, tone) { #name, tone },

constexpr Note k_noteTable[] = {
    N(g, 1275)
    N(a, 1136)
    N(b, 1014)
    N(c, 956)
    N(d, 851)
};

// ISRs signature: no input, void return
void ISRPlayPause();
TFT_eSPI g_tft;

// TODO: add in local function IoTHub_Init();

// microcontrollers are single-threaded; gotta rely on frameworks for concurrency
// TODO: see https://github.com/microsoft/IoT-For-Beginners/blob/395a23b34636e5196a88690886dcd1e20bc8eef9/2-farm/lessons/4-migrate-your-plant-to-the-cloud/code/wio-terminal/soil-moisture-sensor/src/main.cpp


constexpr uint32_t k_landscape_width{TFT_HEIGHT}; //320
constexpr uint32_t k_landscape_height{TFT_WIDTH}; //240
constexpr uint32_t k_char_pixel_width{12}; //eyeballed
constexpr uint32_t k_char_pixel_height{15}; //eyeballed
constexpr uint32_t k_height_offset_absolute{k_char_pixel_height}; // for more picture space

void displayLineCentered(const char* str, const uint32_t currentLineIndex, const uint32_t maxLineCount)
{
    const uint32_t width_offset = k_landscape_width / 2 - strlen(str) / 2 * k_char_pixel_width;
    const uint32_t final_width = max(width_offset, 0u);

    const uint32_t height_offset = k_landscape_height / 2 - maxLineCount * k_char_pixel_height / 2 - k_height_offset_absolute;
    const uint32_t final_height = height_offset + currentLineIndex * k_char_pixel_height;
    g_tft.drawString(str, final_width, final_height);
}

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

static bool g_isPlaying = true;
static int g_playerPosition = 0;

void setup()
{
    //set buzzer pin as output
    pinMode(BUZZER_PIN, OUTPUT);

    Serial.begin(115200);
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(WIO_KEY_A), ISRPlayPause, LOW);

    g_tft.begin();
    //g_tft.setRotation(1); // for development
    g_tft.setRotation(3); // for demo

    g_tft.fillScreen(TFT_RED);

    g_tft.setTextColor(TFT_BLUE);
    g_tft.setTextSize(2);

    const char* strArray[] = {
        "Merry Christmas and",
        "Happy New Years!",
        "Stay warm and drink",
        "hot chocolate <3",
    };

    const uint32_t arraySize = sizeof(strArray) / sizeof(strArray[0]);

    for (uint32_t i = 0u; i < arraySize; ++i) {
        displayLineCentered(strArray[i], i, arraySize);
    }

    // outer mug handle
    g_tft.drawCircle(225, 205, 15, TFT_YELLOW);
    g_tft.fillCircle(225, 205, 15, TFT_YELLOW);
    // inner mug handle
    g_tft.drawCircle(225, 205, 5, TFT_YELLOW);
    g_tft.fillCircle(225, 205, 5, TFT_RED);
    // mug
    g_tft.drawRect(200, 185, 25, 45, TFT_YELLOW);
    g_tft.fillRect(200, 185, 25, 45, TFT_YELLOW);

    // left steam line
    g_tft.drawCircle(205, 180, 5, TFT_WHITE);
    g_tft.fillRect(205, 175, 6, 10, TFT_RED);
    g_tft.drawCircle(205, 170, 5, TFT_WHITE);
    g_tft.fillRect(200, 165, 6, 10, TFT_RED);
    // right steam line
    g_tft.drawCircle(220, 180, 5, TFT_WHITE);
    g_tft.fillRect(220, 175, 6, 10, TFT_RED);
    g_tft.drawCircle(220, 170, 5, TFT_WHITE);
    g_tft.fillRect(215, 165, 6, 10, TFT_RED);
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

void loop()
{
    playMusic();
}

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

