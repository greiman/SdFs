/*
 * Example use of two SPI ports on an STM32 board.
 * Note SPI speed is limited to 18 MHz.
 */
#include <SPI.h>
#include "SdFs.h"
#include "FreeStack.h"

// Chip select PA4, shared SPI, 18 MHz, port 1.
#define SD1_CONFIG SdSpiConfig(PA4, SHARED_SPI, SD_SCK_MHZ(18), 1) 
SdFs sd1;
FsFile dir1;
FsFile file1;

// Chip select PB21, dedicated SPI, 18 MHz, port 2.
#define SD2_CONFIG SdSpiConfig(PB12, DEDICATED_SPI, SD_SCK_MHZ(18), 2) 
SdFs sd2;
FsFile dir2;
FsFile file2;

const uint8_t BUF_DIM = 100;
uint8_t buf[BUF_DIM];

const uint32_t FILE_SIZE = 1000000;
const uint16_t NWRITE = FILE_SIZE/BUF_DIM;
//------------------------------------------------------------------------------
// print error msg, any SD error codes, and halt.
// store messages in flash
#define error(msg) {Serial.println(msg); errorHalt();}
void errorHalt() {
  if (sd1.sdErrorCode()) {
    Serial.print("sd1 errorCode: 0x");
    Serial.println(sd1.sdErrorCode(), HEX);
  }
  if (sd2.sdErrorCode()) {
    Serial.print("sd2 errorCode: 0x");
    Serial.println(sd2.sdErrorCode(), HEX);    
  }
  while (true) {}
}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  // Wait for USB Serial 
  while (!Serial) {
    SysCall::yield();
  }
  Serial.print(F("FreeStack: "));
  Serial.println(FreeStack());

  // fill buffer with known data
  for (size_t i = 0; i < sizeof(buf); i++) {
    buf[i] = i;
  }

  Serial.println(F("type any character to start"));
  while (!Serial.available()) {
    SysCall::yield();
  }

  // initialize the first card
  if (!sd1.begin(SD1_CONFIG)) {
    error("sd1.begin");
  }
  // create Dir1 on sd1 if it does not exist
  if (!sd1.exists("/Dir1")) {
    if (!sd1.mkdir("/Dir1")) {
      error("sd1.mkdir");
    }
  }
  if (!dir1.open(&sd1, "Dir1", O_READ)) {
     error("dir1.open");   
  }
  // initialize the second card
  if (!sd2.begin(SD2_CONFIG)) {
    error("sd2.begin");
  }
// create Dir2 on sd2 if it does not exist
  if (!sd2.exists("/Dir2")) {
    if (!sd2.mkdir("/Dir2")) {
      error("sd2.mkdir");
    }
  }
  if (!dir2.open(&sd2, "Dir2", O_READ)) {
     error("dir2.open");   
  }  
  // remove test.bin from /Dir1 directory of sd1
  if (dir1.exists("test.bin")) {
    if (!dir1.remove("test.bin")) {
      error("remove test.bin");
    }
  }
  // remove rename.bin from /Dir2 directory of sd2
  if (dir2.exists("rename.bin")) {
    if (!dir2.remove("rename.bin")) {
      error("remove rename.bin");
    }
  }
  // list directories.
  Serial.println(F("------sd1 Dir1-------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 Dir2-------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));

  // create or open /Dir1/test.bin and truncate it to zero length
  if (!file1.open(&dir1, "test.bin", O_RDWR | O_CREAT | O_TRUNC)) {
    error("file1.open");
  }
  Serial.println(F("Writing test.bin to sd1"));

  // write data to /Dir1/test.bin on sd1
  for (uint16_t i = 0; i < NWRITE; i++) {
    if (file1.write(buf, sizeof(buf)) != sizeof(buf)) {
      error("file1.write");
    }
  }

  // create or open /Dir2/copy.bin and truncate it to zero length
  if (!file2.open(&dir2, "copy.bin", O_WRITE | O_CREAT | O_TRUNC)) {
    error("file2.open");
  }
  Serial.println(F("Copying test.bin to copy.bin"));

  // copy file1 to file2
  file1.rewind();
  uint32_t t = millis();

  while (1) {
    int n = file1.read(buf, sizeof(buf));
    if (n < 0) {
      error("file1.read");
    }
    if (n == 0) {
      break;
    }
    if ((int)file2.write(buf, n) != n) {
      error("file2.write");
    }
  }
  t = millis() - t;
  Serial.print(F("File size: "));
  Serial.println(file2.fileSize());
  Serial.print(F("Copy time: "));
  Serial.print(t);
  Serial.println(F(" millis"));
  // close test.bin
  file1.close();
  // sync copy.bin so ls works.
  file2.sync(); 
  // list directories.
  Serial.println(F("------sd1 -------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 -------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));
  Serial.println(F("Renaming copy.bin"));
  // Rename copy.bin. The renamed file will be in Dir2.
  if (!file2.rename(&dir2, "rename.bin")) {
    error("rename copy.bin");
  }
  file2.close();
  // list directories.
  Serial.println(F("------sd1 -------"));
  sd1.ls("/", LS_R | LS_SIZE);
  Serial.println(F("------sd2 -------"));
  sd2.ls("/", LS_R | LS_SIZE);
  Serial.println(F("---------------------"));
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void loop() {}