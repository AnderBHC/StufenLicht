#include <NeoPixelBus.h>
#include <esp_dmx.h>
#include <Arduino.h>

#define DMXStart 1
#define PinStripA 27
#define NumPixelA 100
#define PinStripB 26
#define NumPixelB 100

#define transmitPin 17
#define recievePin 16
#define enablePin 21
dmx_port_t dmxPort = 2;
byte data[DMX_MAX_PACKET_SIZE];
QueueHandle_t queue;

uint8_t Red = 0;
uint8_t Blue = 0;
uint8_t Green = 0;
uint8_t Mode = 0;
uint8_t Speed = 0;
uint32_t lastShow = 0;
byte blikstate = 0;
uint8_t progress = 0;

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> StripA (NumPixelA, PinStripA);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0800KbpsMethod> StripB (NumPixelB, PinStripB);

void setup(){
  dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
  dmx_param_config(dmxPort, &dmxConfig);
  dmx_set_pin(dmxPort, transmitPin, recievePin, enablePin);

  StripA.Begin();
  StripB.Begin();

  StripA.Show();
  StripB.Show();
}

void loop(){
  dmx_event_t packet;
  if(xQueueReceive(queue, &packet, DMX_RX_PACKET_TOUT_TICK)){
    if(packet.status == DMX_OK){
      dmx_read_packet(dmxPort, data, packet.size);
      Red = data[DMXStart];
      Green = data[DMXStart + 1];
      Blue = data[DMXStart + 2];
      Mode = data[DMXStart + 3];
      Speed = data[DMXStart +4];
    }
  }
  else{
    Red = 0;
    Green = 0;
    Blue = 0;
    Mode = 0;
    Speed = 0;
  }
  uint8_t* PixelsA = StripA.Pixels();
  uint8_t* PixelsB = StripB.Pixels();

  //Static
  if (Mode >= 0 && Mode < 63){
      for (int i = 0; i < NumPixelA; i++){
        PixelsA[i * 3] = Red;
        PixelsA[i * 3 + 1] = Green;
        PixelsA[i * 3 + 2] = Blue;
      }
      for (int i = 0; i < NumPixelB; i++){
        PixelsB[i * 3] = Red;
        PixelsB[i * 3 + 1] = Green;
        PixelsB[i * 3 + 2] = Blue;
      }
      StripA.Dirty();
      StripB.Dirty();
  }
  //Blinken
//   else if (Mode >= 1 && Mode <2){
//     if (millis() - lastTime > (255-Speed)*4){
//       lastTime = millis();
//       if (blinkstate == 0){
//           blinkstate = 1;
//       }
//       else{
//         blinkstate = 0;
//       }
//     }
//     for (i = 0; i < NumPixelA; i++){
//       StripA.setPixel(i,Red*blinkstate,Blue*blinkstate,Green*blinkstate);
//     }
//     for (i = 0; i <NumPixelB; i++){
//       StripB.setPixel(i,Red*blinkstate,Blue*blinkstate,Green*blinkstate);
//     }
//   }
//   //Rainbow
//   else if (Mode >=2 && Mode < 3){
//     uint32_t offset = millis() * Speed >> 10;
//     for(int i = 0; i < NumPixelA; i++){
//       StripA.setPixel(i,RainbowRot(offset+i),RainbowGruen(offset+i),RainbowBlau(offset+i));
//     }
//     for(int i = 0; i < NumPixelB; i++){
//       StripB.setPixel(i,RainbowRot(offset+i),RainbowGruen(offset+i),RainbowBlau(offset+i));
//     }
//   }
//   //Pulse
//   else if (Mode>= 3 && Mode<4){
//     if millis()-lastTime > (256-Speed){
//       progress++;
//     }
//     for ( int i = 0; i++; i < NumPixelA){
//       if (i < (progress*NumPixelA >> 8)-NumPixelA/4 || i > progress*NumPixelA>>8){
//         StripA.setPixel(i,0,0,0);
//       }
//       else {
//         int hue = 255*4/NumPixelA*progress+255 * (1 - 4 * progress / NumPixelA)
//         StripA.setPixel(i,hue*Red>>8 , hue*Blue >> 8, hue* Green >> 8);
//       }
//     }
//     for ( int i = 0; i++; i < NumPixelB){
//       if (i < (progress*NumPixelB >> 8)-NumPixelB/4 || i > progress*NumPixelB>>8){
//         StripB.setPixel(i,0,0,0);
//       }
//       else {
//         int hue = 255*4/NumPixelB*progress+255 * (1 - 4 * progress / NumPixelB)
//         StripB.setPixel(i,hue*Red>>8 , hue*Blue >> 8, hue* Green >> 8);
//       }
//     }
//   }
  if (millis() - lastShow > 42){
    lastShow = millis();
    StripA.Show();
    StripB.Show();
  }
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
