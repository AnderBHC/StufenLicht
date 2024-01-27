#include <Arduino.h>
#include <esp_dmx.h>
#include <NeoPixelBus.h>
#include <Preferences.h>

Preferences preferences;

int receivePin = 3;
int transmitPin = 1;
int enablePin = 4;

dmx_port_t dmxPort = 1;
byte data[DMX_MAX_PACKET_SIZE];
QueueHandle_t queue;
unsigned int timer = 0;
bool dmxIsConnected = false;

uint8_t Red = 0;
uint8_t Blue = 0;
uint8_t Green = 0;
uint8_t Mode = 0;
uint8_t Speed = 0;
uint32_t lastShow = 0;

int32_t lastBlink = 0;
byte blinkState = 0;
uint8_t progress = 0;

NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt0Sk6812Method> StripA (NumPixelA, PinStripA);
NeoPixelBus<NeoGrbFeature, NeoEsp32Rmt1Sk6812Method> StripB (NumPixelB, PinStripB);


void setup(){
  Serial.begin(115200);
  preferences.begin("DMXPreferences",RO_MODE);
  if !isKey("DMXStartAddr")
    preferences.begin("DMXPreferences", RW_MODE);
    //first boot enter Web config
  }
  else{
    DMXStart = preferences.getUShort("DMXStartAddr");
    NumPixelA = preferences.getUInt("NumPixelStipA");
    NumPixelB = preferences.getUInt("NumPixelStipB");
    NumPixelC = preferences.getUInt("NumPixelStripC");
    NumPixelD = preferences.getUInt("NumPixelStripD");
  }

  dmx_config_t dmxConfig = DMX_DEFAULT_CONFIG;
  dmx_param_config(dmxPort, &dmxConfig);
  dmx_set_pin(dmxPort, transmitPin, recievePin, enablePin);
  int queueSize = 1;
  int interruptPriority = 1;
  dmx_driver_install(dmxPort, DMX_MAX_PACKET_SIZE, queueSize, &queue, interruptPriority);

  StripA.Begin();
  StripB.Begin();

  StripA.Show();
  StripB.Show();
}

void loop(){
  dmx_event_t packet;
  if(xQueueReceive(queue, &packet, DMX_RX_PACKET_TOUT_TICK)){
    if(packet.status == DMX_OK){
      if (!dmxIsConnected){
        Serial.println("DMX connected!");
        dmxIsConnected = true;
      }
      dmx_read_packet(dmxPort, data, packet.size);
      timer += packet.duration;

      Red = data[DMXStart];
      Green = data[DMXStart + 1];
      Blue = data[DMXStart + 2];
      Mode = data[DMXStart + 3];
      Speed = data[DMXStart +4];

      if (timer >= 1000000){
        Serial.printf("Start code is 0x%02X and slot 4 is 0x%02X\n", data[0], data[4]);
        timer -= 1000000;
      }
    }
    else{
      Serial.println("DMX error!");
    }
  }
  else if(dmxIsConnected){
    Red = 0;
    Green = 0;
    Blue = 0;
    Mode = 0;
    Speed = 0;
    Serial.println("DMX timed out!");
  }
  uint8_t* PixelsA = StripA.Pixels();
  uint8_t* PixelsB = StripB.Pixels();

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
      StripA.Dirty();
      StripB.Dirty();
  }
  //Blinken
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
  StripA.Show();
  StripB.Show();
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
