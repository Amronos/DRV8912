#include "Arduino.h"
#include "DRV89xx.h"
#include "DRV89xxRegister.h"

DRV89xx::DRV89xx(int cs_pin, int fault_pin, int sleep_pin, int sck_pin, int miso_pin, int mosi_pin) :  
  _spi_settings(4000000, MSBFIRST, SPI_MODE1), 
  _cs_pin(cs_pin), _fault_pin(fault_pin), _sleep_pin(sleep_pin),
  _sck_pin(sck_pin), _miso_pin(miso_pin), _mosi_pin(mosi_pin) {}

void DRV89xx::begin() {
  Serial.println("DRV89xx begin called");
  if (begin_called_) return;  // ignore duplicate begin calls
  begin_called_ = true;

  // Setup SPI
  int sck = (_sck_pin == -1) ? SCK : _sck_pin;
  int miso = (_miso_pin == -1) ? MISO : _miso_pin;
  int mosi = (_mosi_pin == -1) ? MOSI : _mosi_pin;
  SPI.begin(sck, miso, mosi, -1);

  // Setup pins
  pinMode(_cs_pin, OUTPUT);
  digitalWrite(_cs_pin, HIGH); // Idle high
  if(_sleep_pin) {
    pinMode(_sleep_pin, OUTPUT);
    digitalWrite(_sleep_pin, HIGH);  // enable chip
    delay(1);                        // Give it plenty of time to wake up
  }
  if(_fault_pin) pinMode(_fault_pin, INPUT);

  // Configure device
  _config_cache[(int)DRV89xxRegister::OLD_CTRL_1] = 0x00; // Enable OLD on all channels
  
  // OLD_OP = 1 (Keep bridges active during OLD), OLD_REP = 1 (Don't scream on nFAULT for OLD)
  _config_cache[(int)DRV89xxRegister::OLD_CTRL_2] = 0b11000000; 
  
  _config_cache[(int)DRV89xxRegister::OLD_CTRL_3] = 0b10000000; // set OCP deglitch to 60us
  _config_cache[(int)DRV89xxRegister::OLD_CTRL_4] = 0x00;
  _config_cache[(int)DRV89xxRegister::SR_CTRL_1] = 0xFF; // Set slew rate to 2.5us
  _config_cache[(int)DRV89xxRegister::SR_CTRL_2] = 0xFF; 
  _config_cache[(int)DRV89xxRegister::PWM_FREQ_CTRL] = 0xFF;  // Set all 4 PWM channels to 2kHz
  
  // Clear the mandatory Power-On-Reset (POR) flag to enable outputs
  writeRegister((byte)DRV89xxRegister::CONFIG_CTRL, 0x01); // CLR_FLT = 1
  delay(1);
  writeConfig(); // Send all cached config
}

void DRV89xx::configMotor(byte motor_id, byte hb1, byte hb2, byte pwm_channel, byte reverse_delay) {
  config_changed_ = true;
  _motor[motor_id] = DRV89xxMotor(hb1, hb2, pwm_channel, reverse_delay);
}

byte DRV89xx::writeRegister(byte address, byte value) {
  SPI.beginTransaction(_spi_settings);
  digitalWrite(_cs_pin, LOW);
  uint16_t ret = SPI.transfer16((address << 8) | value);
  digitalWrite(_cs_pin, HIGH);
  SPI.endTransaction();
  delayMicroseconds(5);  // Satisfy tSC_SPI (2.5us) + buffer
  return ret & 0xFF;
}

byte DRV89xx::readRegister(byte address) {
  SPI.beginTransaction(_spi_settings);
  digitalWrite(_cs_pin, LOW);
  uint16_t ret = SPI.transfer16(DRV89xx_REGISTER_READ | (address << 8));
  digitalWrite(_cs_pin, HIGH);
  SPI.endTransaction();
  delayMicroseconds(5);
  return ret & 0xFF;
}


void DRV89xx::readErrorStatus(bool print, bool reset) {
  byte ic_stat = readRegister(0x00);

  if (print) {
    bool fault_pin_active = (digitalRead(_fault_pin) == 0);
    printf("--- DRV8912 Status ---\n");
    printf("nFAULT Pin: %s\n", fault_pin_active ? "LOW (FAULT)" : "HIGH (OK)");
    printf("IC Status (0x00): %02X (POR bit: %d, Fault: %s)\n", 
           ic_stat, (ic_stat & 0x01), (ic_stat > 1) ? "YES" : "NO");
    
    byte ocp1 = readRegister(0x01);
    byte ocp2 = readRegister(0x02);
    byte old1 = readRegister(0x04);
    byte old2 = readRegister(0x05);
    
    if (ocp1 || ocp2) printf("OVERCURRENT detected: %02X %02X\n", ocp1, ocp2);
    if (old1 || old2) printf("OPEN LOAD detected: %02X %02X\n", old1, old2);
    printf("----------------------\n");
  }
  
  if ((((ic_stat & 0x01) == 0) || digitalRead(_fault_pin) == 0) && reset) {
    writeRegister((byte)DRV89xxRegister::CONFIG_CTRL, 0x01);
    writeConfig();
  } 
}

void DRV89xx::writeConfig() {
  for(byte i=DRV89xx_CONFIG_WRITE_START; i<DRV89xx_CONFIG_BYTES; i++) {
    // Only write actual register values (skipping the gap between 0x1B and 0x24)
    if (i <= 0x1B || i == 0x24) {
      writeRegister(i, _config_cache[i]);
    }
  }
}

void DRV89xx::updateConfig() {
  if (!config_changed_) return;
  config_changed_ = false;

  readErrorStatus(false, true);

  for(byte i=0; i<DRV89xx_MAX_MOTORS; i++) {
    _motor[i].applyConfig(_config_cache);
  }
  
  // Update full config range to ensure OLD_OP and other settings are active
  writeConfig();
}
