#include <stdlib.h>  
#include <stdio.h>  
#include <linux/i2c-dev.h>  
#include <linux/i2c-dev.h>  
#include <fcntl.h>    
#include <linux/i2c.h>
  
#define SLAVE_ADDRESS 0xD0  
  
// I2C Linux device handle  
int g_i2cFile;  
    
// open the Linux device  
void i2cOpen()  
{  
  g_i2cFile = open("/dev/i2c-0", O_RDWR);  
  if (g_i2cFile < 0) {  
  perror("i2cOpen");  
  puts("I2C Open Failed");  
  exit(1);  
  }  
  else  
  puts("I2C Open Success");  
}  
  
// close the Linux device  
void i2cClose()  
{  
  close(g_i2cFile);  
}  
  
// set the I2C slave address for all subsequent I2C device transfers  
void i2cSetAddress(int address)  
{  
  if (ioctl(g_i2cFile, I2C_SLAVE, address) < 0) {  
  perror("i2cSetAddress");  
  puts("I2C Set Address Failed");  
  exit(1);  
  }  
  else  
  puts("I2C Set Address Success");  
}  
  
int main(void) {  
  unsigned char res,i;  
  
		puts("Hello I2C World\n"); /* prints Hello World */  
  
		i2cOpen();  
		i2cSetAddress(SLAVE_ADDRESS);  
  
		res = i2c_smbus_read_byte_data(g_i2cFile, 0x5f);  
		printf("Result= %d\n\r\n\r", res);  
  
		i2cClose();  
  
		return 0;  
}
