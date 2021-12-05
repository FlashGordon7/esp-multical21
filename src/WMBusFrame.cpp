/*
 Copyright (C) 2020 chester4444@wolke7.net
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <PubSubClient.h>
#include "WMbusFrame.h"
#include "CRC16.h"
#include "CRC.h"


void  mqttDebug(const char* debug_str);
void  mqttMyData(const char* debug_str);
void  mqttMyDataJson(const char* debug_str);

WMBusFrame::WMBusFrame()
{
  aes128.setKey(key, sizeof(key));
}


void WMBusFrame::check()
{
int16_t crcReg = ((uint16_t)payload[length - 2] << 8) | payload[length-1];
int16_t crcCal = (crc16((uint8_t *) payload, length-2, 0x3D65, 0x0000, 0xFFFF, false, false));
// check meterId

        if (((meterId[3] == payload[4]) && (meterId[2] == payload[5]) && (meterId[1] == payload[6]) && meterId[0] == payload[7]) && (0x44 == payload[1]) && (crcReg == crcCal))// && (0x25 == payload[0])) // 25 skal fjernes
        {

          isValid = true;
           return;
   
        } else {

          isValid = false;
           return;

        }
    


    
}


void WMBusFrame::printMeterInfo(uint8_t *data, size_t len)
{
    // init positions for compact frame  
  int pos_tt = 9; // total consumption
  int pos_tg = 13; // target consumption
  int pos_ic = 7; // info codes
  int pos_ft = 17; // flow temp
  int pos_at = 18; // ambient temp


  if (data[2] == 0x78) // long frame
  {
    // overwrite it with long frame positions
    pos_tt = 10;
    pos_tg = 16;
    pos_ic = 6;
    pos_ft = 22;
    pos_at = 25;
Serial.print("Long frame");
  }
//Serial.printf("%02X", data);
  
  char total[10];
  char mqttstring[25];
  char mqttjsondstring[100];
  String s;
  uint32_t tt = data[pos_tt]
              + (data[pos_tt+1] << 8)
              + (data[pos_tt+2] << 16)
              + (data[pos_tt+3] << 24);
  snprintf(total, sizeof(total), "%d.%03d", tt/1000, tt%1000 );
  //Serial.printf("total: %s m%c - ", total, 179);
  Serial.printf("CurrentValue: %s m3 - ", total);

 // s="/watermeter/MyData";
 // mqttClient.publish(s.c_str(), total, true);
 snprintf(mqttstring, sizeof(mqttstring), "CurrentValue:%d.%03d", tt/1000, tt%1000 );
 mqttMyData(mqttstring);


  char target[10];
  uint32_t tg = data[pos_tg]
              + (data[pos_tg+1] << 8)
              + (data[pos_tg+2] << 16)
              + (data[pos_tg+3] << 24);
  snprintf(target, sizeof(target), "%d.%03d", tg/1000, tg%1000 );
  //Serial.printf("target: %s m%c - ", target, 179);
  Serial.printf("MonthStartValue: %s m3 - ", target);
 snprintf(mqttstring, sizeof(mqttstring), "MonthStartValue:%d.%03d", tg/1000, tg%1000 );
 mqttMyData(mqttstring);

  char flow_temp[3];
  snprintf(flow_temp, sizeof(flow_temp), "%2d", data[pos_ft]);
  //Serial.printf("%s %cC - ", flow_temp, 176);
  Serial.printf("WaterTemp: %s C - ", flow_temp);
  snprintf(mqttstring, sizeof(mqttstring), "WaterTemp:%2d", data[pos_ft]);
  mqttMyData(mqttstring);

  char ambient_temp[3];
  snprintf(ambient_temp, sizeof(ambient_temp), "%2d", data[pos_at]);
  //Serial.printf("%s %cC\n\r", ambient_temp, 176);
  Serial.printf("RoomTemp: %s C\n\r", ambient_temp);
  snprintf(mqttstring, sizeof(mqttstring), "RoomTemp:%2d", data[pos_at]);
  mqttMyData(mqttstring);

snprintf(mqttjsondstring, sizeof(mqttjsondstring), "{\"currentValue\":%d.%03d,\"monthStartValue\":%d.%03d,\"WaterTemp\":%2d,\"RoomTemp\":%2d}",tt/1000, tt%1000, tg/1000, tg%1000, data[pos_ft],data[pos_at]);
mqttMyDataJson(mqttjsondstring);
  

}

 // starting with 1! index 0 is l-field, 1 c-field, 2-3 m-field, 4-9 a-field, 10 crc-field, DATA, Last 2 CRC
    // L-Field: Length Indication 1 byte
    // C-Field: Communication Indication (Request, SEND, RESPONSE EXPECTED, ACK etc...) 1 byte 
    // M-Field: Sending Device Manufacturer ID 2 bytes 
    // A-Field: Address of sending device, consists of { ID number(4 bytes), version(1 byte), device type code(1 byte) } 6 bytes
    // CI-field: Control Information which indicates protocol used at upper layer 1 byte
    // CRC-field: Cyclic Redundancy Check 2 bytes, CRC is calculated from L-field to DATA end. Stop before CRC byte




void WMBusFrame::decode()
{
  // check meterId, CRC
//Serial.println("");
// Serial.println("");
// Serial.print("Checksum: ");
// uint16_t crcReg = ((uint16_t)payload[length - 2] << 8) | payload[length-1];
//  Serial.print((payload[length - 1] << 8) | payload[length]);
CRC16 crc;

//Serial.print(crcReg, HEX);
// Serial.println("");
// Serial.print("Payload: "); 

//    for (uint8_t i = 0; i< ((length)-2); i++)
 //   {
//Serial.printf("%02X", payload[i]);
//      }
//  Serial.println("");
//Serial.print("Calculated checksum: ");
//  Serial.println(crc16((uint8_t *) payload, length-2, 0x3D65, 0x0000, 0xFFFF, false, false), HEX);
  
  check();
  if (!isValid) return;

  uint8_t cipherLength = length - 18; // cipher starts at index 16, remove 2 crc bytes
  memcpy(cipher, &payload[17], cipherLength);

  memset(iv, 0, sizeof(iv));   // padding with 0
  memcpy(iv, &payload[2], 8);
  iv[8] = payload[11];
  memcpy(&iv[9], &payload[13], 4);

  aes128.setIV(iv, sizeof(iv));
  aes128.decrypt(plaintext, (const uint8_t *) cipher, cipherLength);

/*
  Serial.printf("C:     ");
  for (size_t i = 0; i < cipherLength; i++)
  {
    Serial.printf("%02X", cipher[i]);
  }
  Serial.println();
  Serial.printf("P(%d): ", cipherLength);
  for (size_t i = 0; i < cipherLength; i++)
  {
    Serial.printf("%02X", plaintext[i]);
  }
  Serial.println();
*/

  printMeterInfo(plaintext, cipherLength);
}
