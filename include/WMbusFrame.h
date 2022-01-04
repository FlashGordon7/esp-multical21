#ifndef __WMBUS_FRAME__
#define __WMBUS_FRAME__

#include <Arduino.h>
#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include "credentials.h"

#define INFO_CODE_DRY 0x01
#define INFO_CODE_DRY_SHIFT (4+0)

#define INFO_CODE_REVERSE 0x02
#define INFO_CODE_REVERSE_SHIFT (4+3)

#define INFO_CODE_LEAK 0x04
#define INFO_CODE_LEAK_SHIFT (4+6)

#define INFO_CODE_BURST 0x08
#define INFO_CODE_BURST_SHIFT (4+9)

class WMBusFrame
{
  public:
    static const uint8_t MAX_LENGTH = 64;
  private:
    CTR<AESSmall128> aes128;
    const uint8_t meterId[4] = { W_SERIAL_NUMBER }; // Multical21 serial number
    const uint8_t key[16] = { W_ENCRYPTION_KEY }; // AES-128 key
    uint8_t cipher[MAX_LENGTH];
    uint8_t plaintext[MAX_LENGTH];
    uint8_t iv[16];
    void check(void);
    void printMeterInfo(uint8_t *data, size_t len);
    void decodeTime(int time, char* outStr);

  public:
    // check frame and decrypt it
    void decode(void);

    // true, if meter information is valid for the last received frame
    bool isValid = false;

    // payload length
    uint8_t length = 0;

    // payload data
    uint8_t payload[MAX_LENGTH];

    // constructor
    WMBusFrame();
};

#endif // __WMBUS_FRAME__
