/*
  DRV89xx.h - Library for controlling brushed DC motors using a DRV89xx
  Currently written and tested for the ESP32 processor
  Created by Joseph Duchesne, September 23, 2020
  Licensed under BSD 3 Clause 
*/
#ifndef DRV89xx_h
#define DRV89xx_h

#include "Arduino.h"
#include <cstdint>
#include <SPI.h>
#include "DRV89xxMotor.h"

#define DRV89xx_REGISTER_READ 0x4000
#define DRV89xx_MAX_MOTORS 6
#define DRV89xx_CONFIG_WRITE_START 0x7
#define DRV89xx_CONFIG_BYTES 0x25
#define DRV89xx_UPDATE_START 0x08
#define DRV89xx_UPDATE_END 0x24

class DRV89xx
{

  public:
    DRV89xx(int cs_pin, int fault_pin, int sleep_pin, int sck_pin = -1, int miso_pin = -1, int mosi_pin = -1);
    void begin();

    void configMotor(byte motor_id, byte hb1, byte hb2, byte pwm_channel, byte reverse_delay);
    byte writeRegister(byte address, byte value);
    byte readRegister(byte address);
    void readErrorStatus(bool print, bool reset);
    void logStatus();
    void writeConfig();
    void updateConfig();

    void setMotor(byte motor, int16_t pwm){ 
      config_changed_ = true;
      _motor[motor].setPWM(pwm);
    };
    void disableMotor(byte motor){ 
      config_changed_ = true;
      _motor[motor].disable();
    };

    void debugConfig() {
      char buff[32];
      for(int i=DRV89xx_CONFIG_WRITE_START; i<DRV89xx_CONFIG_BYTES; i++) {
        sprintf(buff, "0x%02X: ", i);
        Serial.print(buff);
        for(int j = 7; j >= 0; j--) Serial.print(bitRead( _config_cache[i],j));
        Serial.println();
      }
    };

    
  private:
    SPISettings _spi_settings;
    int _cs_pin, _fault_pin, _sleep_pin;
    int _sck_pin, _miso_pin, _mosi_pin;
    bool begin_called_ = false;
    bool config_changed_ = false;
    byte _config_cache[DRV89xx_CONFIG_BYTES] = {0};  // Fully initalize the config cache as 0
    DRV89xxMotor _motor[DRV89xx_MAX_MOTORS];
};

#endif
