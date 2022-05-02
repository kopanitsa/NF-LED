// By kopanitsa <okada.takahiro111@gmail.com>
// Contains code (c) Adafruit, license BSD

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

static const uint16_t PROGMEM
    RGB_bmp[64] =
      { 0x100, 0x200, 0x300, 0x400, 0x600, 0x800, 0xA00, 0xF00, 
        0x101, 0x202, 0x303, 0x404, 0x606, 0x808, 0xA0A, 0xF0F, 
        0x001, 0x002, 0x003, 0x004, 0x006, 0x008, 0x00A, 0x00F, 
        0x011, 0x022, 0x033, 0x044, 0x066, 0x088, 0x0AA, 0x0FF, 
        0x010, 0x020, 0x030, 0x040, 0x060, 0x080, 0x0A0, 0x0F0, 
        0x110, 0x220, 0x330, 0x440, 0x660, 0x880, 0xAA0, 0xFF0, 
        0x100, 0x200, 0x300, 0x400, 0x600, 0x800, 0xA00, 0xF00, 
        0x111, 0x222, 0x333, 0x444, 0x666, 0x888, 0xAAA, 0xFFF,
};

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(8, 8, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h, uint8_t x_offset) {
    static uint16_t *RGB_bmp_fixed = (uint16_t *) malloc( w*h*2);
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
        uint8_t r,g,b;
        uint16_t color = pgm_read_word(bitmap + pixel);
        b = (color & 0xF00) >> 8;
        g = (color & 0x0F0) >> 4;
        r = color & 0x00F;
        b = map(b, 0, 15, 0, 31);
        g = map(g, 0, 15, 0, 63);
        r = map(r, 0, 15, 0, 31);
        RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
    }
    x = x + x_offset;
    if (x <= w) {
        matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
    } else {
        matrix->drawRGBBitmap(x - x_offset - 1, y, RGB_bmp_fixed, w, h);
    }
    
}

void display_rgbBitmap(uint8_t x_offset) { 
    matrix->clear();
    fixdrawRGBBitmap(0, 0, RGB_bmp, MATRIX_WIDTH, MATRIX_HEIGHT, x_offset);
    matrix->show();
}

void setup() {
  matrix->begin();
  matrix->setTextWrap(false);
//  matrix->setBrightness(40);
}

int x = 0;
void loop() {
  display_rgbBitmap(x);
  delay(500);
  x++;
  if (x >= MATRIX_WIDTH) {
    x = 0;
  }
}
