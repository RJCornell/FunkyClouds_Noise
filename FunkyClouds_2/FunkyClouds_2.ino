// Mini Funky Clouds
// Modified by Russ Cornell for Matrix:
// Funky Clouds 2016
// Matrix Effects for WS2812B Leds (30x8) 
// www.stefan-petrick.de/wordpress_beta
// https://gist.github.com/anonymous/68298debac462330719b
    
#include <FastLED.h>
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

// your matrix size if it is NOT a 16*16
// check in that case the setup part and
// RenderCustomMatrix() and
// ShowFrame() for more comments
const uint8_t CUSTOM_WIDTH = 30;
const uint8_t CUSTOM_HEIGHT = 8;

// LED Setup
#define LED_PIN     6
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define BRIGHTNESS  80

// all 2D effects will be calculated in this matrix size
// do not touch
const uint8_t WIDTH = 16;
const uint8_t HEIGHT = 16;

#define NUM_LEDS (WIDTH * HEIGHT)

// the rendering buffer (16*16)
CRGB leds[NUM_LEDS];

// your display buffer for any setup except 16*16
CRGB leds2[CUSTOM_HEIGHT * CUSTOM_WIDTH];

// Oscillator Setup
struct timer {
  unsigned long tact;         
  unsigned long lastMillis; 
  unsigned long count; 
  int delta; 
  byte up; 
  byte down; 
};
timer multiTimer[7];
int timers = sizeof(multiTimer) / sizeof(multiTimer[0]); 

void setup() {
  // LED Setup
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
  
  // Triangular Oscillator Setup
  // range (up/down), speed (tact=ms between steps) 
  multiTimer[0].tact=  70;     //x1
  multiTimer[0].up = WIDTH - 1;
  multiTimer[0].down = 0;

  multiTimer[1].tact=  50;     //y1
  multiTimer[1].up = HEIGHT - 1;
  multiTimer[1].down = 0;
  
  multiTimer[2].tact=  3;      //color
  multiTimer[2].up = 255;
  multiTimer[2].down = 0;
  
  multiTimer[3].tact=  76;     //x2  
  multiTimer[3].up = WIDTH - 1;
  multiTimer[3].down = 0;
  
  multiTimer[4].tact=  99;     //y2
  multiTimer[4].up = HEIGHT - 1;
  multiTimer[4].down = 0;
  
  multiTimer[5].tact=  73;    //center spiral x
  multiTimer[5].up = HEIGHT - 4;
  multiTimer[5].down = 0;
  
  multiTimer[6].tact=  145;    //center spiral y
  multiTimer[6].up = HEIGHT - 4;
  multiTimer[6].down = 0;
  
  // set counting directions positive for the beginning
  // and start with random values to keep the start interesting
  
  for (int i = 0; i < timers; i++) {
    multiTimer[i].delta = 1;
    multiTimer[i].count = random(multiTimer[i].up); 
  }
}

void loop() {
  // let the oscillators swing
  UpdateTimers();
  
/*  
  // draw just a line defined by 5 oscillators
  Line(
    multiTimer[3].count,  // x1
    multiTimer[4].count,  // y1
    multiTimer[0].count,  // x2
    multiTimer[1].count,  // y2
    multiTimer[2].count   // color
  );
*/  

  // the "seed": 3 moving dots
    leds[XY(multiTimer[0].count,multiTimer[1].count)] = 
    CHSV (multiTimer[2].count , 255, 255);
  
  leds[XY(multiTimer[3].count,multiTimer[4].count)] = 
    CHSV (255-multiTimer[2].count , 255, 255);
    
  // coordinates are the average of 2 oscillators
  leds[XY((
    multiTimer[0].count+multiTimer[1].count)/2, 
    (multiTimer[3].count+multiTimer[4].count)/2 
    )] = CHSV (multiTimer[2].count/2 , 255, 255);

  
  // the balance of the (last) values of the following functions affects the
  // appearence of the effect a lot
  
  // a moving spiral
  SpiralStream(multiTimer[5].count, multiTimer[6].count, 3, 210); // play here

  // y wind
  StreamVertical(120);    // and here
  
  // x wind
  StreamHorizontal(110);  // and here
   
  // main spiral
  SpiralStream(6,7,8,150);// and here
      
  // increase the contrast
  //DimmAll(250);
  
  // done.
  FastLED.show();
}

// finds the right index for a S shaped matrix
int XY(int x, int y) { 
  if(y > HEIGHT) { y = HEIGHT; }
  if(y < 0) { y = 0; }
  if(x > WIDTH) { x = WIDTH;} 
  if(x < 0) { x = 0; }
  if(x % 2 == 1) {  
  return (x * (WIDTH) + (HEIGHT - y -1));
  } else { 
    // use that line only, if you have all rows beginning at the same side
    return (x * (WIDTH) + y);  
  }
}

// counts everything with different speeds linear up and down
// = oscillators following a triangular function
void UpdateTimers() {
  unsigned long now=millis();
  for (int i=0; i < timers; i++) 
  {
    while (now-multiTimer[i].lastMillis >= multiTimer[i].tact)
    {
      multiTimer[i].lastMillis += multiTimer[i].tact;
      multiTimer[i].count = multiTimer[i].count + multiTimer[i].delta;
      if ((multiTimer[i].count == multiTimer[i].up) || (multiTimer[i].count == multiTimer[i].down)) 
      {
        multiTimer[i].delta = -multiTimer[i].delta;
      }
    }
  }
}

// fade the image buffer arround
// x, y: center   r: radius   dimm: fade down
void SpiralStream(int x,int y, int r, byte dimm) {  
  for(int d = r; d >= 0; d--) {                // from the outside to the inside
    for(int i = x-d; i <= x+d; i++) {
       leds[XY(i,y-d)] += leds[XY(i+1,y-d)];   // lowest row to the right
       leds[XY(i,y-d)].nscale8( dimm );}
    for(int i = y-d; i <= y+d; i++) {
       leds[XY(x+d,i)] += leds[XY(x+d,i+1)];   // right colum up
       leds[XY(x+d,i)].nscale8( dimm );}
    for(int i = x+d; i >= x-d; i--) {
       leds[XY(i,y+d)] += leds[XY(i-1,y+d)];   // upper row to the left
       leds[XY(i,y+d)].nscale8( dimm );}
    for(int i = y+d; i >= y-d; i--) {
       leds[XY(x-d,i)] += leds[XY(x-d,i-1)];   // left colum down
       leds[XY(x-d,i)].nscale8( dimm );}
  }
}
    
// Bresenham line algorythm
void Line(int x0, int y0, int x1, int y1, byte color) {
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2; 
  for(;;){  
    leds[XY(x0, y0)] += CHSV(color, 255, 255);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; } 
    if (e2 < dx) { err += dx; y0 += sy; } 
  }
}

// scale the brightness of the screenbuffer down
void DimmAll(byte value) {
  for(int i = 0; i < NUM_LEDS; i++) 
  {
    leds[i].nscale8(value);
  }
}

// give it a linear tail
void StreamHorizontal(byte scale) {
  for(int x = 1; x < WIDTH ; x++) {
    for(int y = 0; y < HEIGHT; y++) {
      leds[XY(x,y)] += leds[XY(x-1,y)];
      leds[XY(x,y)].nscale8( scale );
    }
  }
  for(int y = 0; y < HEIGHT; y++) 
    leds[XY(0,y)].nscale8(scale);
}

// give it a linear tail
void StreamVertical(byte scale) {
  for(int x = 0; x < WIDTH ; x++) {
    for(int y = 1; y < HEIGHT; y++) {
      leds[XY(x,y)] += leds[XY(x,y-1)];
      leds[XY(x,y)].nscale8( scale );
    }
  }
  for(int x = 0; x < WIDTH; x++) 
    leds[XY(x,0)].nscale8(scale);
}

