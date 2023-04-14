#ifndef _SIGMA_STUDIO_H_
#define _SIGMA_STUDIO_H_

typedef uint8_t ADI_REG_TYPE;

extern void SIGMA_WRITE_REGISTER_BLOCK(uint8_t DEVICE_ADDR, uint16_t SUB_ADDR, uint32_t len, ADI_REG_TYPE* data );
extern void SIGMA_WRITE_DELAY(uint8_t DEVICE_ADDR,  uint16_t len, ADI_REG_TYPE* data );
#endif


