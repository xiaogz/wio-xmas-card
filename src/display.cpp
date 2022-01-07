#include <TFT_eSPI.h>

static TFT_eSPI g_tft;

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

void drawMug()
{
    constexpr int k_mugTopCornerX = 200;
    constexpr int k_mugTopCornerY = 185;

    // outer mug handle
    g_tft.drawCircle(k_mugTopCornerX + 25, k_mugTopCornerY + 20, 15, TFT_YELLOW);
    g_tft.fillCircle(k_mugTopCornerX + 25, k_mugTopCornerY + 20, 15, TFT_YELLOW);
    // inner mug handle
    g_tft.drawCircle(k_mugTopCornerX + 25, k_mugTopCornerY + 20, 5, TFT_YELLOW);
    g_tft.fillCircle(k_mugTopCornerX + 25, k_mugTopCornerY + 20, 5, TFT_RED);
    // mug
    g_tft.drawRect(k_mugTopCornerX, k_mugTopCornerY, 25, 45, TFT_YELLOW);
    g_tft.fillRect(k_mugTopCornerX, k_mugTopCornerY, 25, 45, TFT_YELLOW);

    // left steam line
    g_tft.drawCircle(k_mugTopCornerX + 5, k_mugTopCornerY - 5, 5, TFT_WHITE);
    g_tft.fillRect(k_mugTopCornerX + 5, k_mugTopCornerY - 10, 6, 10, TFT_RED);
    g_tft.drawCircle(k_mugTopCornerX + 5, k_mugTopCornerY - 15, 5, TFT_WHITE);
    g_tft.fillRect(k_mugTopCornerX, k_mugTopCornerY - 20, 6, 10, TFT_RED);
    // right steam line
    g_tft.drawCircle(k_mugTopCornerX + 20, k_mugTopCornerY - 5, 5, TFT_WHITE);
    g_tft.fillRect(k_mugTopCornerX + 20, k_mugTopCornerY - 10, 6, 10, TFT_RED);
    g_tft.drawCircle(k_mugTopCornerX + 20, k_mugTopCornerY - 15, 5, TFT_WHITE);
    g_tft.fillRect(k_mugTopCornerX + 15, k_mugTopCornerY - 20, 6, 10, TFT_RED);
}

void setupDisplay()
{
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

    drawMug();
}

