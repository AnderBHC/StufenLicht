#include <Arduino.h>
#include <esp_dmx.h>
#include <NeoPixelBus.h>
#include <Preferences.h>

#define RW_MODE false
#define RO_MODE true

Preferences data;

int recievePin = 3;
int transmitPin = 1;
int enablePin = 4;

dmx_port_t dmxPort = 0;
byte DMXdata[5];
bool dmxIsConnected = false;
unsigned long lastUpdate = millis();
QueueHandle_t queue;


uint16_t DMXStart;
uint16_t NumPixelA;
uint16_t NumPixelB;
uint16_t NumPixelC;
uint16_t NumPixelD;

uint8_t PinStripA = 26;
uint8_t PinStripB = 25;
uint8_t PinStripC = 32;
uint8_t PinStripD = 33;


uint8_t Red = 0;
uint8_t Blue = 0;
uint8_t Green = 0;
uint8_t Mode = 0;
uint8_t Speed = 0;
uint32_t lastShow = 0;

int32_t lastBlink = 0;
byte blinkState = 0;
uint8_t progress = 0;

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Sk6812Method>* StripA = NULL;
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Sk6812Method>* StripB = NULL;
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2Sk6812Method>* StripC = NULL;
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt3Sk6812Method>* StripD = NULL;

void setup(){
  data.begin("DMXPreferences", RO_MODE);
  if (!data.isKey("DMXStartAddr")){
    data.begin("DMXPreferences", RW_MODE);
    //first boot enter Web config
  }
  else{
    DMXStart = data.getUInt("DMXStartAddr");
    NumPixelA = data.getUInt("NumPixelStipA");
    NumPixelB = data.getUInt("NumPixelStipB");
    NumPixelC = data.getUInt("NumPixelStripC");
    NumPixelD = data.getUInt("NumPixelStripD");
  }
  StripA = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Sk6812Method>(NumPixelA, PinStripA);
  StripB = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Sk6812Method>(NumPixelB, PinStripB);
  StripC = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt2Sk6812Method>(NumPixelC, PinStripC);
  StripD = new NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt3Sk6812Method>(NumPixelD, PinStripD);

  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_driver_install(dmxPort, &config, DMX_INTR_FLAGS_DEFAULT);
  dmx_set_pin(dmxPort, transmitPin, recievePin, enablePin);

  StripA->Begin();
  StripB->Begin();
  StripC->Begin();
  StripD->Begin();

  StripA->Show();
  StripB->Show();
  StripC->Show();
  StripD->Show();
}

void loop(){
  dmx_packet_t packet;
  if(dmx_receive(dmxPort, &packet, DMX_TIMEOUT_TICK)){

    if(!packet.err){
      if (!dmxIsConnected){
        dmxIsConnected = true;
        //DMX is cinnected!
      }
      dmx_read_offset(dmxPort, DMXStart, DMXdata, 5);
      Red = DMXdata[DMXStart];
      Green = DMXdata[DMXStart + 1];
      Blue = DMXdata[DMXStart + 2];
      Mode = DMXdata[DMXStart + 3];
      Speed = DMXdata[DMXStart +4];
    }
  }
  else{
    Red = 0;
    Green = 0;
    Blue = 0;
    Mode = 0;
    Speed = 0;
  }
  uint8_t* PixelsA = StripA->Pixels();
  uint8_t* PixelsB = StripB->Pixels();
  uint8_t* PixelsC = StripC->Pixels();
  uint8_t* PixelsD = StripD->Pixels();
  //Static
  if (Mode < 64){
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
      for (int i = 0; i < NumPixelC; i++){
        PixelsC[i * 3] = Red;
        PixelsC[i * 3 + 1] = Green;
        PixelsC[i * 3 + 2] = Blue;
      }
      for (int i = 0; i < NumPixelD; i++){
        PixelsD[i * 3] = Red;
        PixelsD[i * 3 + 1] = Green;
        PixelsD[i * 3 + 2] = Blue;
      }
      StripA->Dirty();
      StripB->Dirty();
      StripC->Dirty();
      StripD->Dirty();
  }
/*  //Blinken
  else if (Mode >= 64 && Mode < 128){
    int calc = -6 * Speed + 1750;
    if (blinkState == 0 && millis() - lastBlink > 500){
      blinkState == 1;
      lastBlink = millis();

    }
    else if (blinkState == 1 && millis() - lastBlink > 250){
      blinkState = 0;
      lastBlink = millis();

    }

    for (int i = 0; i < NumPixelA; i++){
      PixelsA[i * 3] = Red * blinkState;
      PixelsA[i * 3 + 1] = Green * blinkState;
      PixelsA[i * 3 + 2] = Blue * blinkState;
    }
    for (int i = 0; i < NumPixelB; i++){
      PixelsB[i * 3] = Red * blinkState;
      PixelsB[i * 3 + 1] = Green * blinkState;
      PixelsB[i * 3 + 2] = Blue * blinkState;
    }
    StripA.Dirty();
    StripB.Dirty();
  }
  */
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
//     if millis()-lastBlink > (256-Speed){
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
  StripA->Show();
  StripB->Show();
  StripC->Show();
  StripD->Show();
  }

// byte RainbowRot(unsigned int offset){
//   int c = offset%766;
//   if (c >= 0 && c <= 255){
//     return 255 - c;
//   }
//   if (c > 255 && c <= 510) {
//     return 0;
//   }
//   if (c > 510){
//     return c - 510;
//   }
// }
// byte RainbowGruen(unsigned int offset){
//   int c = offset%766;
//   byte color;
//   if (c >= 0 && c <= 255){
//     return c;
//   }
//   if (c > 255 && c <= 510){
//     return 510 - c;
//   }
//   if(c > 510){
//     return 0;
//   }
// }
// byte RainbowBlau(unsigned int offset){
//   int c = offset%766;
//   if(c >= 0 && c <= 255){
//     return 0;
//   }
//   if (c>=256 && c<510){
//     return c - 255;
//   }
//   if (c >= 510){
//     return 765 - c;
//   }
// }
