#include <linux/i2c-dev.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>



int main()
{
	int file;
	int addr = 0x48; /* The I2C address */
	int adapter_nr = 0; /* probably dynamically determined */
  	char filename[20];
  
	//printf("11111111111111111111111111111111");
  	snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
  	file = open(filename, O_RDWR);
  	if (file < 0) {
    	/* ERROR HANDLING; you can check errno to see what went wrong */
    	exit(1);
  	}

	//printf("222222222222222222222222222222222");  

  if (ioctl(file, I2C_SLAVE, addr) < 0) {
    /* ERROR HANDLING; you can check errno to see what went wrong */
	printf("SLAVE ADDRESS SETTING FAILED");
    exit(1);
  }

	//printf("333333333333333333333333333333333");  
  
   __u8 reg = 0x5f; /* Device register to access */
  __s32 res;
  char buf[10];

  /* Using SMBus commands */
 // res = i2c_smbus_read_word_data(file, reg);
  if (res < 0) {
    /* ERROR HANDLING: i2c transaction failed */
  } else {
  //  printf("Read word : %d \n",res);
  }
//	printf("4444444444444444444444444444444444");  

  /* Using I2C Write, equivalent of 
     i2c_smbus_write_word_data(file, reg, 0x6543) */
  buf[0] = reg;
  buf[1] = 0x44;
  buf[2] = 0x66;
  if (write(file, buf, 3) != 3) {
    /* ERROR HANDLING: i2c transaction failed */
  }
//	printf("5555555555555555555555555555555555");  

  /* Using I2C Read, equivalent of i2c_smbus_read_byte(file) */
  if (read(file, buf, 1) != 1) {
    /* ERROR HANDLING: i2c transaction failed */
    perror("read");
  } else {
    printf("Got the Result : %02x  %02x %02x \n",buf[0],buf[1],buf[2]);
  }
//  	printf("666666666666666666666666666666666666");  

}


