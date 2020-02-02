#include "LoRa.h"
#include "Config.h"
#include "KISS.h"
bool commandMode = true;
char line_buffer[128];
int bufc = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(serialBaudRate);
  while (!Serial); // Waiting until LoRa32u4 is ready

  // Buffers
  memset(rxBuffer, 0, sizeof(rxBuffer));
  memset(txBuffer, 0, sizeof(txBuffer));

  LoRa.setPins(pinNSS, pinNRST, pinDIO0);

  startRadio();
}

bool startRadio() {
  if (!LoRa.begin(loraFrequency)) {
    kissIndicateError(ERROR_INITRADIO);
    Serial.println("FAIL");
    while(1);
  }
  else {
    LoRa.setSpreadingFactor(loraSpreadingFactor);
    LoRa.setCodingRate4(loraCodingRate);
    LoRa.setSignalBandwidth(bandWidthTable[loraBandwidth]);
    //LoRa.enableCrc();
    LoRa.onCadDone(cadCallback);
    LoRa.onReceive(receiveCallback);
    LoRa.receive();
  }
}

void transmit(size_t size) {
  size_t written = 0;

  if (size > MTU) {
    size = MTU;
  }

  LoRa.beginPacket();
  for (size_t i; i < size; i++) {
    LoRa.write(txBuffer[i]);
    written++;
  }
  LoRa.endPacket();
  LoRa.receive();
}

void serialCallback(uint8_t txByte) {
  if (inFrame && txByte == FEND) {
    inFrame = false;
    if ( command == CMD_DATA ) {
       //Serial.println("FULL_KISS");
       if (outboundReady) {
         kissIndicateError(ERROR_QUEUE_FULL);
      } else {
        outboundReady = true;
        //Serial.println("RDY_OUT");
      }
    } else if ( command == CMD_RETURN ) {
      commandMode = true;
    }
  }
  else if (txByte == FEND) {
    //Serial.println("KISS_FLAG");
    inFrame = true;
    command = CMD_UNKNOWN;
    frameLength = 0;
  }
  else if (inFrame && frameLength < MTU) {
    // Get command byte
    if (frameLength == 0 && command == CMD_UNKNOWN) {
      //Serial.println("ACQ_CMD");
      command = txByte;
    }
    else if (command == CMD_DATA) {
      if (txByte == FESC) {
        escape = true;
      }
      else {
        if (escape) {
          if (txByte == TFEND) {
            txByte = FEND;
          }
          if (txByte == TFESC) {
            txByte = FESC;
          }
          escape = false;
        }
        else {
          txBuffer[frameLength++] = txByte;
        }
      }
    }
  }
}

bool isOutboundReady() {
  return outboundReady;
}

void cadCallback(boolean cadDetected) {
  if (cadDetected) {
    lastHeard = millis();
    channelBusy = true;
    LoRa.receive();
  } else {
    LoRa.CAD();
  }
  return;
}

void receiveCallback(int packetSize) {
  readLength = 0;
  lastRssi = LoRa.packetRssi();
  lastSnr = LoRa.packetSnr();
  getPacketData(packetSize);

  // Send RSSI
  Serial.write(FEND);
  Serial.write(CMD_HARDWARE);
  Serial.print(lastRssi);
  Serial.write(',');
  Serial.print(lastSnr);
  Serial.write(FEND);

  // And then write the entire packet
  Serial.write(FEND);
  Serial.write((uint8_t)CMD_DATA);
  for (int i = 0; i < readLength; i++) {
    uint8_t temp = rxBuffer[i];
    if (temp == FEND) {
      Serial.write(FESC);
      temp = TFEND;
    }
    if (temp == FESC) {
      Serial.write(FESC);
      temp = TFESC;
    }
    Serial.write(temp);
  }
  Serial.write(FEND);
  readLength = 0;
  lastHeard = millis();
  channelBusy = false;
  LoRa.receive();
}

void escapedSerialWrite (uint8_t bufferByte) {
  switch(bufferByte) {
    case FEND:
      Serial.write(FESC);
      Serial.write(TFEND);
      break;
    case FESC:
      Serial.write(FESC);
      Serial.write(TFESC);
      break;
    default:
      Serial.write(bufferByte);
  }
}

void kissIndicateError(uint8_t errorCode) {
  Serial.write(FEND);
  Serial.write(CMD_ERROR);
  Serial.write(errorCode);
  Serial.write(FEND);
}

void getPacketData(int packetLength) {
  while (packetLength--) {
    rxBuffer[readLength++] = LoRa.read();
  }
}

void do_command(char buffer[]) {
  char *cmd;
  uint32_t f;
  int bw,sf,cr,backoff;

  cmd = strtok(buffer," ");
  Serial.println(cmd);
  if ( strcasecmp(cmd,"set") == 0) {
    f = atol(strtok(NULL,","));
    bw = atoi(strtok(NULL,","));
    sf = atoi(strtok(NULL,","));
    cr = atoi(strtok(NULL,","));
    backoff = atoi(strtok(NULL,","));

    Serial.println("LoRa Parameters:");

    f = f * 10000;
    Serial.print("Freq=");
    Serial.println(f);
    if (f < freq_low || f > freq_high) {
      Serial.println("Out of band.");
      return;
    }
    loraFrequency = f;
    LoRa.setFrequency(loraFrequency);

    Serial.print("BW=");
    Serial.println(bw);
    if (bw < 0 || bw > 8) {
      Serial.println("unsupported bandwidth.");
      return;
    }
    loraBandwidth = bw;
    LoRa.setSignalBandwidth(bandWidthTable[loraBandwidth]);

    Serial.print("SF=");
    Serial.println(sf);
    if (sf < 6 || sf > 12) {
      Serial.println("unsupported spredingfactor.");
      return;
    }
    loraSpreadingFactor = sf;
    LoRa.setSpreadingFactor(loraSpreadingFactor);

    Serial.print("CR=");
    Serial.println(cr);
    if (cr < 5 || cr > 8) {
      Serial.println("unsupported codingrate.");
      return;
    }
    loraCodingRate = cr;
    LoRa.setCodingRate4(loraCodingRate);

    Serial.print("Backoff=");
    Serial.println(backoff);
    if (backoff < 500) {
      Serial.println("backtime error.");
      return;
    }
    loraMaxBackoff = backoff;
    Serial.println(F("TNC MODE."));
    commandMode = false;
  } else {
    Serial.print(F("Unknown command:"));
    Serial.println(buffer);
  }
}
void loop() {
   uint32_t now = millis();
   if (isOutboundReady() && !SERIAL_READING) {
    if (now - backofft > backoffDuration) {
      if (!channelBusy && (now - lastHeard > lbtDuration)) {
        outboundReady = false;
        transmit(frameLength);
        backofft = 0;
        backoffDuration = 0;
        lastHeard = 0;
      } else {
        backofft = now;
        if (backoffDuration > 0)
          backoffDuration = backoffDuration /2;
        else 
          backoffDuration = random(lbtDuration , loraMaxBackoff);
      }
    }
  }  
  if (Serial.available()) {
    SERIAL_READING = true;
    char txByte = Serial.read();
    if (commandMode) {
     if (txByte == '\r') {
      line_buffer[bufc] = '\0';
      do_command(line_buffer);
      line_buffer[0] = '\0';
      bufc = 0;
     } else {
      line_buffer[bufc++] =  txByte;
      if (bufc > 128)
       bufc = 127;
     }
    } else {
      serialCallback(txByte);
    }
    lastSerialRead = millis();
  } else {
    if (SERIAL_READING && millis() - lastSerialRead >= serialReadTimeout) {
      SERIAL_READING = false;
    }
  }
}
