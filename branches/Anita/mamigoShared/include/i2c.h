/*
 * i2c.h
 *
 *  Created on: 21-Nov-2009
 *      Author: mamigo
 */

#ifndef I2C_H_
#define I2C_H_

int i2cWrite2ByteToByteAddress(char* device, unsigned char client, unsigned char reg, unsigned short value);
int i2cRead2ByteFromByteAddress(char* device, unsigned char client, unsigned char reg);
int i2cDumpRegShort(char* device, unsigned char client, unsigned short start, unsigned short end);
void i2cScanBus(char* device);
int i2cReadByte(char* device, unsigned char client);
int i2cWriteByte(char* device, unsigned char client, unsigned char value);
int i2cWriteByteOnByteAddres(char* device, unsigned char client, unsigned char reg, unsigned char value);
int i2cReadByteFromByteAddress(char* device, unsigned char client, unsigned char reg);


#endif /* I2C_H_ */
