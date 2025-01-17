#include "WS_Flow.h"

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT (8*8)

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, RGB_Control_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void Strip_Init() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  strip.setPixelColor(0, 128, 128, 128);
  strip.setPixelColor(7, 128, 0, 0);
  strip.setPixelColor(63, 0, 0, 128);
  strip.show();
}

enum Side {
  UNKNOWN = 9,
  FRONT = 1,
  BACK  = 3,
  LEFT  = 2,
  RIGHT = 0
};

Adafruit_NeoMatrix Matrix = Adafruit_NeoMatrix(8, 8, RGB_Control_PIN, 
  NEO_MATRIX_TOP     + NEO_MATRIX_RIGHT +                    
  NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,                
  NEO_GRB            + NEO_KHZ800);                       

int MatrixWidth = 0;
const uint16_t colors[] = {
  Matrix.Color(255, 0, 0), Matrix.Color(0, 255, 0), Matrix.Color(0, 0, 255) };

int getCharWidth(char c) {
  //if (c == 'i' || c == 'l' || c == '!' || c == '.') {
  //  return 3;
  // } else if (c == 'm' || c == 'w') {
  //   return 7;
  //} else {
    return 5;
  //}
}
int getStringWidth(const char* str) {
  int width = 0;
  int length = strlen(str);
  printf("%d\r\n",length);
  for (int i = 0; i < length; i++) {
    width += getCharWidth(str[i]);
    width += 1;      
  }
  printf("%d\r\n",width);
  return width;
}
void Matrix_Init() {
  Matrix.begin();
  Matrix.setTextWrap(false);  
  
  // English: Please note that the brightness of the lamp bead should not be too high, which can easily cause the temperature of the board to rise rapidly, thus damaging the board !!!
  // Chinese: 请注意，灯珠亮度不要太高，容易导致板子温度急速上升，从而损坏板子!!! 
  Matrix.setBrightness(40);           // set brightness
  Matrix.setTextColor(colors[0]); 
  MatrixWidth   = Matrix.width(); 
}
void Text_Flow(char* Text) {
  Matrix.setRotation(FRONT);
  int textWidth   = getStringWidth(Text);
  Matrix.fillScreen(0);
  Matrix.setCursor(MatrixWidth, 0);
  Matrix.print(F(Text));
  if (--MatrixWidth < -textWidth) {
    MatrixWidth = Matrix.width();
  }
  Matrix.show();
}
