#ifndef __I2C_H
#define __I2C_H

#define I2C_CR2_FREQ_MASK	0x3ff
#define I2C_CCR_CCRMASK	0xfff
#define I2C_TRISE_MASK	0x3f

void i2c_init(void);
void i2c_deinit(void);
uint8_t i2c_write(uint32_t i2c, uint8_t address, uint8_t reg,
	uint8_t data);
int i2c_read(uint32_t i2c, uint8_t address, uint8_t reg);


#endif
