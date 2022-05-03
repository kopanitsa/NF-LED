// By kopanitsa <okada.takahiro111@gmail.com>
// Contains code (c) Adafruit, license BSD

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

#define PIN 6

static const uint16_t PROGMEM
    RGB_bmp[64] =
      { 
0x528, 0xe29, 0xd05, 0x086, 0xba9, 0x305, 0x346, 0x1cb, 
0x1e7, 0x5eb, 0x7f0, 0x395, 0x433, 0xc03, 0xe76, 0xa41, 
0x198, 0x788, 0x6af, 0x767, 0xd7a, 0x9, 0x57f, 0x188, 
0x7a8, 0xBF1, 0x9b1, 0x4fC, 0x0dF, 0x6Fd, 0x9B2, 0xacc,
0x528, 0xe29, 0xd05, 0x086, 0xba9, 0x305, 0x346, 0x1cb, 
0x1e7, 0x5eb, 0x7f0, 0x395, 0x433, 0xc03, 0xe76, 0xa41, 
0x198, 0x788, 0x6af, 0x767, 0xd7a, 0x9, 0x57f, 0x188, 
0x7a8, 0xBF1, 0x9b1, 0x4fC, 0x0dF, 0x6Fd, 0x9B2, 0xacc,
};

static uint32_t RAMDOM_SEED = 0x9Af1;
static uint8_t INITIAL_DIR = 0x4; // 0~7
static uint8_t DIR_CHANGE_PERCENT = 10;
static uint16_t INITIAL_SPEED = 0x7e; // 0~255 -> x5msec
static uint32_t SPEED_DIFF_RANGE = 30;
static uint32_t SPEED_MIN = 50;
static uint32_t SPEED_MAX = 2000;

int8_t curr_dir = 0;
uint16_t curr_speed = 100;
int dir_x = 0;
int dir_y = 0;

static uint16_t bmp_slide[64] = {0};

#define MATRIX_WIDTH 8
#define MATRIX_HEIGHT 8

Adafruit_NeoMatrix *matrix = new Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
  NEO_GRB            + NEO_KHZ800);

void slide(int8_t x_offset, int8_t y_offset, const uint16_t* src) {
  uint8_t dx, dy = 0;
  for (int i=0; i<MATRIX_WIDTH; i++) {
    for (int j=0; j<MATRIX_HEIGHT; j++) {
      if (i-x_offset < 0) {
        dx = i-x_offset+MATRIX_WIDTH;
      } else {
        dx = i-x_offset;
      }
      if (j-y_offset < 0) {
        dy = j-y_offset+MATRIX_HEIGHT;
      } else {
        dy = j-y_offset;
      }
      bmp_slide[dx*8+dy] = pgm_read_word(src + i*8+j);
    }
  }
}

// Convert a BGR 4/4/4 bitmap to RGB 5/6/5 used by Adafruit_GFX
void fixdrawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h) {
    static uint16_t *RGB_bmp_fixed = (uint16_t *) malloc( w*h*2);
    for (uint16_t pixel=0; pixel<w*h; pixel++) {
        uint8_t r,g,b;
        uint16_t color = bitmap[pixel];
        b = (color & 0xF00) >> 8;
        g = (color & 0x0F0) >> 4;
        r = color & 0x00F;
        b = map(b, 0, 15, 0, 31);
        g = map(g, 0, 15, 0, 63);
        r = map(r, 0, 15, 0, 31);
        RGB_bmp_fixed[pixel] = (r << 11) + (g << 5) + b;
    }
    matrix->drawRGBBitmap(x, y, RGB_bmp_fixed, w, h);
}

void display_rgbBitmap(uint8_t x_offset, uint8_t y_offset) { 
    matrix->clear();
    slide(x_offset, y_offset, RGB_bmp);
    fixdrawRGBBitmap(0, 0, bmp_slide, MATRIX_WIDTH, MATRIX_HEIGHT);
    matrix->show();
}

void update_dir() {
  int p = random(100);
  if (p < DIR_CHANGE_PERCENT) {
    // change dir
    if (p%2) {
      // CW
      curr_dir++;
      if (curr_dir > 7) {curr_dir = 0;}
    } else {
      // CCW
      curr_dir--;
      if (curr_dir < 0) {curr_dir = 7;}
    }
  }

  switch (curr_dir) {
    case 0:
      dir_x = dir_x;
      dir_y = dir_y+1;
      break;  
    case 1:
      dir_x = dir_x+1;
      dir_y = dir_y+1;
      break;
    case 2:
      dir_x = dir_x+1;
      dir_y = dir_y;
      break;
    case 3:
      dir_x = dir_x+1;
      dir_y = dir_y-1;
      break;
    case 4:
      dir_x = dir_x;
      dir_y = dir_y-1;
      break;
    case 5:
      dir_x = dir_x-1;
      dir_y = dir_y-1;
      break;
    case 6:
      dir_x = dir_x-1;
      dir_y = dir_y;
      break;
    case 7:
      dir_x = dir_x-1;
      dir_y = dir_y+1;
      break;
  }

  if (dir_x >= MATRIX_WIDTH) {
    dir_x = 0;
  } else if (dir_x < 0) {
    dir_x = MATRIX_WIDTH - 1;
  }
  if (dir_y >= MATRIX_HEIGHT) {
    dir_y = 0;
  } else if (dir_y < 0) {
    dir_y = MATRIX_HEIGHT - 1;
  }
  Serial.print("dir:");
  Serial.println(curr_dir);
//  Serial.print("dir_x:");
//  Serial.println(dir_x);
//  Serial.print("dir_y:");
//  Serial.println(dir_y);
}

void update_speed() {
  int d = random(1, SPEED_DIFF_RANGE) - SPEED_DIFF_RANGE/2;
  curr_speed += d;
  if (curr_speed >= SPEED_MAX) {
    curr_speed = SPEED_MAX;
  }
  if (curr_speed <= SPEED_MIN) {
    curr_speed = SPEED_MIN;
  }  
//  Serial.print("speed:");
//  Serial.println(curr_speed);
}

void setup() {
  Serial.begin(9600);
  Serial.println("start");
  randomSeed(RAMDOM_SEED);

  curr_speed = INITIAL_SPEED * 5;
  curr_dir = INITIAL_DIR % 7;
  
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(40);
}

void loop() {
  update_dir();
  update_speed();
  display_rgbBitmap(dir_x, dir_y);

  delay(curr_speed);
}
