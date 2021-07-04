#include <Adafruit_NeoPixel.h>
#include DMXSerial
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define DMXStart 1
#define PinStripA 3
#define NumPixelA 100
#define PinStripB 4
#define NumPixelB 100
int Red = 0;
int Blue = 0;
int Green = 0;
int Mode = 0;
int Speed = 0;
uint32_t lastTime = 0;
byte blikstate = 0;

Adafruit_NeoPixel StripA (NumPixelA, PinStripA, NEO_RGB + NEOKHZ800);
Adafruit_NeoPixel StripB (NumPixelB, PinStripB, NEO_RGB + NEOKHZ800);
uint8_t *RXBuffer;
void setup(){
  DMXSerial.init(DMXReciver);
  RXBuffer = DMXSerial.getBuffer();
  DMXSerial.setStartAddress(DMXStart);
  StripA.begin();
  StripB.begin();

  StripA.clear();
  StripB.clear();

  StripA.show();
  StripB.show();
}

void loop(){
  if (DMXSerial.recive()){
    Red = RXBuffer[0];
    Blue = RXBuffer[1];
    Green = RXBuffer[2];
    Mode = RXBuffer[3];
    Speed = RXBuffer[4];
  }
  //Static
  if (Mode >= 0 && Mode < 1){
      for (i = 0; i < NumPixelA; i++){
        StripA.setPixel(i,Red,Blue,Green);
      }
      for (i = 0; i <NumPixelB; i++){
        StripB.setPixel(i,Red,Blue,Green);
      }
  }
  //Blinken
  else if (Mode >= 1 && Mode <2){
    if (millis() - lastTime > (255-Speed)*4){
      lastTime = millis();
      if (blinkstate == 0){
          blinkstate = 1;
      }
      else{
        blinkstate = 0;
      }
    }
    for (i = 0; i < NumPixelA; i++){
      StripA.setPixel(i,Red*blinkstate,Blue*blinkstate,Green*blinkstate);
    }
    for (i = 0; i <NumPixelB; i++){
      StripB.setPixel(i,Red*blinkstate,Blue*blinkstate,Green*blinkstate);
    }
  }
  //Rainbow
  else if (Mode >=2 && Mode < 3){
    uint32_t offset = millis() * Speed >> 10;
    for(int i = 0; i < NumPixelA; i++){
      StripA.setPixel(i,RainbowRot(offset+i),RainbowGruen(offset+i),RainbowBlau(offset+i));
    }
    for(int i = 0; i < NumPixelB; i++){
      StripB.setPixel(i,RainbowRot(offset+i),RainbowGruen(offset+i),RainbowBlau(offset+i));
    }
  }
  //Pulse


  StripA.show();
  StripB.show();
}
byte RainbowRot(unsigned int offset){
  int c = offset%766;
  if (c >= 0 && c <= 255){
    return 255 - c;
  }
  if (c > 255 && c <= 510) {
    return 0;
  }
  if (c > 510){
    return c - 510;
  }
}
byte RainbowGruen(unsigned int offset){
  int c = offset%766;
  byte color;
  if (c >= 0 && c <= 255){
    return c;
  }
  if (c > 255 && c <= 510){
    return 510 - c;
  }
  if(c > 510){
    return 0;
  }
}
byte RainbowBlau(unsigned int offset){
  int c = offset%766;
  if(c >= 0 && c <= 255){
    return 0;
  }
  if (c>=256 && c<510){
    return c - 255;
  }
  if (c >= 510){
    return 765 - c;
  }
}
