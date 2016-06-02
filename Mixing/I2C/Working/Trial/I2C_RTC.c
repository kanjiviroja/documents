



//***************************************************************************************
//***************************************************************************************
/// this program reads the RTC's seconds counter continously and displays the lower nibble
//  on the LED's on the ESDK board.
//  also on the nios terminal the status is printed.
//***************************************************************************************
//***************************************************************************************




#include <stdio.h>
#include "unistd.h"
#include "io.h"
#include "system.h"

int *prescale_low, *prescale_high, *control_reg, *transmit_reg;
int *recieve_reg, *command_reg, *status_reg,*led_out;


void delay(int count);
void checkTIP();

int main()

{

    int temp;

    prescale_low   =(int*) (I2C_MASTER_0_BASE + 0x00000000);
    prescale_high  =(int*) (I2C_MASTER_0_BASE + 0x00000004);
    control_reg    =(int*) (I2C_MASTER_0_BASE + 0x00000008);
    transmit_reg   =(int*) (I2C_MASTER_0_BASE + 0x0000000c);
    command_reg    =(int*) (I2C_MASTER_0_BASE + 0x00000010);
    recieve_reg    =(int*) (I2C_MASTER_0_BASE + 0x0000000c);
    status_reg     =(int*) (I2C_MASTER_0_BASE + 0x00000010);
    led_out        =(int*) 0x00041080;



    *control_reg   = 0x00;      //Enable the core & disabel the intrrupt
    *prescale_low  = 0x00;      //set the prescale reg for 100KHz I2C speed
    *prescale_high = 0x00;


    *prescale_low  = 0x5f;      //set the prescale reg for 100KHz I2C speed
    *prescale_high = 0x00;
    *control_reg   = 0x80;      //Enable the core & disabel the intrrupt


        *transmit_reg  = 0xD0;
        *command_reg   = 0x90;

    checkTIP();
        *led_out=0x01;

        *transmit_reg  = 0x00;
        *command_reg   = 0x10;

    checkTIP();

        *transmit_reg  = 0x00;
        *command_reg   = 0x50;   //send write with stop

    checkTIP();

        *transmit_reg  = 0xD0;
        *command_reg   = 0x90;

    checkTIP();

        *transmit_reg  = 0x07;
        *command_reg   = 0x10;

    checkTIP();

        *transmit_reg  = 0x90;
        *command_reg   = 0x50;  //send write with stop

    checkTIP();

        while(1)
        {
            *transmit_reg  = 0xD0;    //I2C slave(RTC) address
            *command_reg   = 0x90;    //command for start & write

          checkTIP();

            *transmit_reg  = 0x00;    //set the address counter of slave to 00
            *command_reg   = 0x10;    //command for write

          checkTIP();

            *transmit_reg  = 0xD1;      //slave address for read
            *command_reg   = 0x90;   //command for write

          checkTIP();

            *command_reg   = 0x28;    //command for read and NAK

          checkTIP();

            usleep(250000);
            temp = *recieve_reg;     //read the recive reg
            *led_out = ~(temp);

            printf("\nReadData Second :  %x", temp);
        }

   return 0;
}

void checkTIP()
{
          int t;
          t= *status_reg & 0x2;
         while(t== 0x2)
          {
           t= *status_reg & 0x2;
           if(t == 0x0)
            break;

      }

}

