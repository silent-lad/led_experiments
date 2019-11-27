#include <FastLED.h>

#define LED_PIN     3
#define NUM_LEDS    30
#define BRIGHTNESS  255
//#define LED_TYPE    WS2812B
//#define COLOR_ORDER GRB
#define LED_TYPE    UCS1903
#define COLOR_ORDER BRG
CRGB leds[NUM_LEDS];


void setup() {
    Serial.begin(9600);
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    CRGB purple = CHSV( HUE_PURPLE, 0, 255);
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255,20,57);
    }
    FastLED.show();
}


void loop(){
    static int red = 100;
    static int green= 100;
    static int blue= 255;
    
    Serial.print(0);
    Serial.print("\t");
    Serial.print(255);
    Serial.print("\t");
    int R = inoise8(red++);
    Serial.print(R);
    Serial.print("\t");
    int G = inoise8(green--);
    Serial.println(G);
//    Serial.print("\t");
//    int B = inoise8(blue++);
//    Serial.println(B);

    uint8_t brightness = 255;

    for( int i = 1; i < NUM_LEDS; i++) {
        leds[i] = leds[i-1];
    }
    leds[0]=CRGB(R/5,G/5,blue);
    
//    for( int i = 0; i < NUM_LEDS; i++) {
//        leds[i] = CRGB(R,G,B);
//    }
    FastLED.show();
    delay(10);
    
}