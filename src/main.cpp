#include <SPI.h>
#include <TFT_eSPI.h>

#include <music.h>

// Button map when looking at 3 buttons with screen facing up
// button 1 - a innermost
// button 2 - b between 1a and 3c
// button 3 - c outermost edge

constexpr int k_ISRDebounceMs = 200;

// ISRs signature: no input, void return
void ISRPlayPause();
TFT_eSPI g_tft;

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

