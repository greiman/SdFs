/*
 * This program is a simple binary write/read benchmark.
 *
 * If SD_FAT_TYPE == 2, exFAT, the file will be
 * pre-allocated for improved write latency.
 */
#include "SdFs.h"
#include "FreeStack.h"

// SD_FAT_TYPE = 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 1
/*
  Change the value of SD_CS_PIN if you are using SPI and
  your hardware does not use the default value, SS.
  Common values are:
  Arduino Ethernet shield: pin 4
  Sparkfun SD shield: pin 8
  Adafruit SD shields and modules: pin 10
*/

// SDCARD_SS_PIN is defined for the built-in SD on some boards.
#ifndef SDCARD_SS_PIN
const uint8_t SD_CS_PIN = SS;
#else  // SDCARD_SS_PIN
// Assume built-in SD is used.
const uint8_t SD_CS_PIN = SDCARD_SS_PIN;
#endif  // SDCARD_SS_PIN

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI)
#endif  // HAS_SDIO_CLASS

// If BLOCK_INTERVAL_MICROS is nonzero, a data source that
// produces BUF_SIZE bytes every BUF_INTERVAL_MICROS will
// be simulated. An estimate of the count of buffers required
// in a data logger due to write latency problems will be printed.
const uint32_t BUF_INTERVAL_MICROS = 0;

// Size of read/write.
const size_t BUF_SIZE = 512;

// File size in MB where MB = 1,000,000 bytes.
const uint32_t FILE_SIZE_MB = 5;

// Write pass count.
const uint8_t WRITE_COUNT = 2;

// Read pass count.
const uint8_t READ_COUNT = 2;
//==============================================================================
// End of configuration constants.
//------------------------------------------------------------------------------
// File size in bytes.
const uint32_t FILE_SIZE = 1000000UL*FILE_SIZE_MB;

uint8_t buf[BUF_SIZE];

#if SD_FAT_TYPE == 1
SdFat sd;
File file;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile file;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile file;
#else  // SD_FAT_TYPE
#error Invalid SD_FAT_TYPE
#endif  // SD_FAT_TYPE

// Serial output stream
ArduinoOutStream cout(Serial);
//------------------------------------------------------------------------------
// Store error strings in flash to save RAM.
#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
void cidDmp() {
  cid_t cid;
  if (!sd.card()->readCID(&cid)) {

    error("readCID failed");
  }
  cout << F("\nManufacturer ID: ");
  cout << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) {
    cout << cid.pnm[i];
  }
  cout << F("\nVersion: ");
  cout << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
  cout << F("Serial number: ") << hex << cid.psn << dec << endl;
  cout << F("Manufacturing date: ");
  cout << int(cid.mdt_month) << '/';
  cout << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
  cout << endl;
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);
  cout << F("\nUse a freshly formatted SD for best performance.\n");
  if (!ENABLE_DEDICATED_SPI) {
    cout << F(
      "\nSet ENABLE_DEDICATED_SPI nonzero in\n"
      "SdFs/src/FsConfig.h for best SPI performance.\n"); 
  }

  // use uppercase in hex and use 0X base prefix
  cout << uppercase << showbase << endl;
}
//------------------------------------------------------------------------------
void loop() {
  float s;
  uint32_t t;
  int32_t maxBufMicros = 0;
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;

  // Discard any input.
  do {
    delay(10);
  } while (Serial.available() && Serial.read() >= 0);

  // F() stores strings in flash to save RAM
  cout << F("Type any character to start\n");
  while (!Serial.available()) {
    SysCall::yield();
  }

  cout << F("FreeStack: ") << FreeStack() << endl;

  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
  if (sd.fatType() == EXFAT_TYPE) {
    cout << F("Type is exFAT") << endl;
  } else {
    cout << F("Type is FAT") << int(sd.fatType()) << endl;
  }

  cout << F("Card size: ") << sd.card()->sectorCount()*512E-9;
  cout << F(" GB (GB = 1E9 bytes)") << endl;

  cidDmp();

  // open or create file - truncate existing file.
  if (!file.open("bench.dat", O_CREAT | O_TRUNC | O_RDWR)) {
    error("open failed");
  }

  // fill buf with known data
  for (uint16_t i = 0; i < (BUF_SIZE-2); i++) {
    buf[i] = 'A' + (i % 26);
  }
  buf[BUF_SIZE-2] = '\r';
  buf[BUF_SIZE-1] = '\n';

  cout << F("FILE_SIZE_MB = ") << FILE_SIZE_MB << endl;
  cout << F("BUF_SIZE = ") << BUF_SIZE << F(" bytes\n");
  if (BUF_INTERVAL_MICROS) {
    cout << F("BUF_INTERVAL_MICROS = ") << BUF_INTERVAL_MICROS << endl;
  }
  cout << F("Starting write test, please wait.") << endl << endl;

  // do write test
  uint32_t n = FILE_SIZE/sizeof(buf);
  cout <<F("write speed and latency") << endl;
  cout << F("speed,max,min,avg") << endl;
  cout << F("KB/Sec,usec,usec,usec") << endl;
  for (uint8_t nTest = 0; nTest < WRITE_COUNT; nTest++) {
    file.truncate(0);
#if SD_FAT_TYPE == 2
  if (!file.preAllocate(FILE_SIZE)) {
    error("preAllocate failed");
  }
#endif  // SD_FAT_TYPE == 2
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    t = millis();
    uint32_t bm = micros();
    for (uint32_t i = 0; i < n; i++) {
      if (BUF_INTERVAL_MICROS) {
        bm += BUF_INTERVAL_MICROS;
        int32_t delta = micros() - bm;
        if (delta > maxBufMicros) {
          maxBufMicros = delta;
        }
        while (delta < 0) {
          delta = micros() - bm;
        }
      }
      uint32_t m = micros();
      if (file.write(buf, sizeof(buf)) != sizeof(buf)) {
        error("write failed");
      }
      m = micros() - m;
      if (maxLatency < m) {
        maxLatency = m;
      }
      if (minLatency > m) {
        minLatency = m;
      }
      totalLatency += m;
    }
    file.sync();
    t = millis() - t;
    s = file.fileSize();
    cout << s/t <<',' << maxLatency << ',' << minLatency;
    cout << ',' << totalLatency/n << endl;
  }
  if (BUF_INTERVAL_MICROS) {
    // Stop spurious gcc division by zero warning.
    size_t neededBufCount = BUF_INTERVAL_MICROS == 0 ? 0 :
                            2 + (maxBufMicros)/BUF_INTERVAL_MICROS;
    cout << F("maxBufMicros: ") << maxBufMicros << endl;
    cout << F("neededBufCount: ") << neededBufCount << endl;
  }
  cout << endl << F("Starting read test, please wait.") << endl;
  cout << endl <<F("read speed and latency") << endl;
  cout << F("speed,max,min,avg") << endl;
  cout << F("KB/Sec,usec,usec,usec") << endl;

  // do read test
  for (uint8_t nTest = 0; nTest < READ_COUNT; nTest++) {
    file.rewind();
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    t = millis();
    for (uint32_t i = 0; i < n; i++) {
      buf[BUF_SIZE-1] = 0;
      uint32_t m = micros();
      int32_t nr = file.read(buf, sizeof(buf));
      if (nr != sizeof(buf)) {
        error("read failed");
      }
      m = micros() - m;
      if (maxLatency < m) {
        maxLatency = m;
      }
      if (minLatency > m) {
        minLatency = m;
      }
      totalLatency += m;
      if (buf[BUF_SIZE-1] != '\n') {
        error("data check error");
      }
    }
    s = file.fileSize();
    t = millis() - t;
    cout << s/t <<',' << maxLatency << ',' << minLatency;
    cout << ',' << totalLatency/n << endl;
  }
  cout << endl << F("Done") << endl;
  file.close();
}