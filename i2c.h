
/**
 *
 * @file i2c.h
 *
 * @author   ms kim
 *
 * @brief
 */

#ifndef I2C_H
#define I2C_H

//Macro
typedef enum {
	TAS3251_DEVICE_ADDR_L = (0x4A), //8bit - 0x94
	TAS3251_DEVICE_ADDR_H = (0x4B) //8bit - 0x96
}TAS3251_ADDR;

typedef enum {
	TAS5806_DEVICE_ADDR_4_7K = (0x2C), //8bit - 0x58
	TAS5806_DEVICE_ADDR_15K = (0x2D), //8bit - 0x5A
	TAS5806_DEVICE_ADDR_47K = (0x2E), //8bit - 0x5C
	TAS5806_DEVICE_ADDR_120K = (0x2F) //8bit - 0x5E
}TAS5806MD_ADDR;

//Function
void I2C_Configure(void);
void I2C_Interrupt_Write_Data(uint8_t uDeviceId, uint8_t uAddr, uint8_t *uData, uint8_t uDataSize);
void I2C_Interrupt_Read_Data(uint8_t uDeviceId, uint8_t uAddr, uint8_t *uData, uint8_t uDataSize);
#endif //I2C_H
