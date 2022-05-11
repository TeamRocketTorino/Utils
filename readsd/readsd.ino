#include <SPI.h>
#include <SdFat.h>
#include "Pins.h"

SdFat sd;
SdFile file;
SdFile dir;

void setMicroSDPower(bool status) {
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, !status);
}

/**
   Initializes micro SD card with exfat filesystem
   @return true if all went well
*/
bool beginSD() {
  bool success = true;
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected

  delay(1);
  setMicroSDPower(true);

  // Max power up time for a standard micro SD is 250ms
  delay(10);

  //Standard SdFat initialization
  if (!sd.begin(PIN_MICROSD_CHIP_SELECT, SD_SCK_MHZ(24))) {

    //Give SD more time to power up, then try again
    delay(250);

    // Trying again standard SdFat initialization
    if (!sd.begin(PIN_MICROSD_CHIP_SELECT, SD_SCK_MHZ(24))) {
      Serial.println("SD init failed (second attempt). Is card present? Formatted?");
      digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH); //Be sure SD is deselected
      success = false;
    }
  }

  // Change to root directory. All new file creation will be in root
  // checking success to only execute chdir if no error occurred so far
  if (success && !sd.chdir()) {
    Serial.println("SD change directory failed");
    success = false;
  }

  return success;
}

/**
   Add CIPO pull-up
   @return true if all went well
*/
bool enableCIPOpullUp() {
  ap3_err_t retval = AP3_OK;
  am_hal_gpio_pincfg_t cipoPinCfg = AP3_GPIO_DEFAULT_PINCFG;
  cipoPinCfg.uFuncSel = AM_HAL_PIN_6_M0MISO;
  cipoPinCfg.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_12MA;
  cipoPinCfg.eGPOutcfg = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL;
  cipoPinCfg.uIOMnum = AP3_SPI_IOM;
  cipoPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  padMode(MISO, cipoPinCfg, &retval);
  return retval == AP3_OK;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  beginSD();
  enableCIPOpullUp();
  dir.open("/", O_READ);
}

void loop() {
  while (!Serial.available());

  String cmd = Serial.readString();
  cmd.trim();

  if (cmd.equals("ls")) {
    while (file.openNext(&dir, O_READ)) {
      file.printName(&Serial);
      Serial.println();
      file.close();
    }
  } else {
    if (file.open(cmd.c_str(), O_READ)) {
      int data;
      while ((data = file.read()) >= 0) {
        Serial.write(data);
      }
      file.close();
    }
  }

  cmd = "";

}
