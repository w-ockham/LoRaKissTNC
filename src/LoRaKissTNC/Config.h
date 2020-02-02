
#ifndef CONFIG_H
  #define CONFIG_H

  #define MAJ_VERS 0x01
  #define MIN_VERS 0x00

  #define MCU_328P 0x90
  #define MCU_32U4 0x91

  #if defined(__AVR_ATmega328P__)
    #define MCU_VARIANT MCU_328P
    #warning "Firmware is being compiled for ATmega328p based boards"
  #elif defined(__AVR_ATmega32U4__)
    #define MCU_VARIANT MCU_32U4
    #warning "Firmware is being compiled for ATmega32u4 based boards"
  #else
    #error "The firmware cannot be compiled for the selected MCU variant"
  #endif
  
  #define MTU  255
  #define CMD_L       4
  int     lastRssi = 0;
  float   lastSnr  = 0;
  uint8_t lastRssiRaw = 0x00;
  
  size_t  readLength = 0;

  #if MCU_VARIANT == MCU_328P
    const int pinNSS = 10;
    const int pinNRST = 9;
    const int pinDIO0 = 2;
    // const int pinLedRx = 5;
    // const int pinLedTx = 4;

  #endif

  #if MCU_VARIANT == MCU_32U4
    const int pinNSS  = 8;
    const int pinNRST = 4;
    const int pinDIO0 = 7;
  #endif

  const long serialBaudRate   = 9600;
  
  int loraMaxBackoff = 10000;
  int lbtDuration = 3000;
  uint32_t backofft;
  uint32_t backoffDuration;
  uint32_t lastHeard = 0;
  boolean outboundReady = false;
  boolean channelBusy = false;
  
  // Default LoRa settings
  int       loraSpreadingFactor = 11;
  int       loraCodingRate      = 8;
  int       loraTxPower         = 20;
  uint32_t  bandWidthTable[] = { 7.8e3, 10.4e3, 15.6e3, 20.8e3, 31.25e3, 41.7e3, 62.5e3, 125e3, 250e3};
  uint32_t  loraBandwidth       = 6;
  const uint32_t freq_low  = 431.0E6;
  const uint32_t freq_high = 439.0E6;
  uint32_t  loraFrequency       = 438.41E6;

  uint8_t txBuffer[MTU];
  uint8_t rxBuffer[MTU];

  uint32_t statRx = 0;
  uint32_t statTx = 0;
  
 
#endif
