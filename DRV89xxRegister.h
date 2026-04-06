#ifndef DRV89xxRegister_h
#define DRV89xxRegister_h

// map for DRV8912/10
enum class DRV89xxRegister : byte {
  IC_STAT = 0x00,
  OCP_STAT_1 = 0x01, 
  OCP_STAT_2 = 0x02, 
  OCP_STAT_3 = 0x03,
  OLD_STAT_1 = 0x04, 
  OLD_STAT_2 = 0x05, 
  OLD_STAT_3 = 0x06,
  CONFIG_CTRL = 0x07,
  OP_CTRL_1 = 0x08, 
  OP_CTRL_2 = 0x09, 
  OP_CTRL_3 = 0x0A,
  PWM_CTRL_1 = 0x0B, 
  PWM_CTRL_2 = 0x0C, 
  FW_CTRL_1 = 0x0D,
  FW_CTRL_2 = 0x0E,
  PWM_MAP_CTRL_1 = 0x0F, 
  PWM_MAP_CTRL_2 = 0x10, 
  PWM_MAP_CTRL_3 = 0x11,
  PWM_FREQ_CTRL = 0x12,
  PWM_DUTY_CTRL_1 = 0x13, 
  PWM_DUTY_CTRL_2 = 0x14, 
  PWM_DUTY_CTRL_3 = 0x15, 
  PWM_DUTY_CTRL_4 = 0x16,
  SR_CTRL_1 = 0x17, 
  SR_CTRL_2 = 0x18,
  OLD_CTRL_1 = 0x19, 
  OLD_CTRL_2 = 0x1A, 
  OLD_CTRL_3 = 0x1B, 
  OLD_CTRL_4 = 0x24
};

#endif
