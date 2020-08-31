#include "stdint.h"
#include "C:/Keil/EE319Kware/inc/tm4c123gh6pm.h"
typedef unsigned int uint32_t;
#define KEYPAD_ROW PORTD //portD from pD0 tO pD3
#define KEYPAD_COLUMN PORTA //portA from pA4 tO pA7
#define Rs 0x20 //at pin 5 portC (pC5)
#define RW 0x40 //at pin 6 portC (pC6)
#define E 0x80 //at pin 7 portC (pC7)
#define LCD_wakeup 0x30
#define LCD_8bitData 0x38
#define LCD_incrementCursor 0x06
#define LCD_clear 0x01
#define LCD_displayOn 0x0F
#define Red 0x02 // pF1
#define Blue 0x04 //pF2
#define Green 0x08 // pF3
#define Yellow 0x0A //pF3 and pF1
#define White 0x0E //pF1, pF2 and pF3
char password[4] = {'0','1','3','0'};
const unsigned char keyButton[4][4]={
	{'1','2','3','A'},
	{'4','5','6','B'},
	{'7','8','9','C'},
	{'*','0','#','D'}};
void SystemInit(){}
void delayMs(int n) //delay for milliseconds(16 MHZ CPU clock)
{
	int i, j;
  for(i = 0 ; i < n; i++)
   for(j = 0; j < 3180; j++){} /*enter loop for 1 ms*/
}
void delayUs(int n) //delay for microseconds(16 MHZ CPU clock)
{
	int i, j;
  for(i = 0 ; i < n; i++)
  for(j = 0; j < 3; j++){} /*enter loop for 1 us*/
}
void keyPad_Init()
{
	SYSCTL_RCGCGPIO_R |=0x08; //set portD for keypad
	while((SYSCTL_RCGCGPIO_R & 0x08)!=0x08){}
	GPIO_PORTD_DIR_R|=0x0F; //KEYPAD OUTPUTS ROW AND 
	GPIO_PORTD_DEN_R|=0x0F; //KEYPAD DIGITAL ROW
	GPIO_PORTD_ODR_R|=0x0F; //KEYPAD OPEN_DRAIN_ROW(protect it from port damage if two keys pressed at same time)
	SYSCTL_RCGCGPIO_R |=0x01; //set portA for keypad
	while((SYSCTL_RCGCGPIO_R & 0x01)!=0x01){}
  GPIO_PORTA_DIR_R |=0x0;	//from pA4 to pA7 as INPUTS COLUMNS
	GPIO_PORTA_DEN_R|=0xF0; //KEYPAD DIGITAL COLUMNS
	GPIO_PORTA_PUR_R|=0xF0; /*KEYPAD PULL_UP_COLUMNS(protect it from floatingPoints)*/
}

void LCD_Command(char com)
{
	GPIO_PORTC_DATA_R=0; //set Rs=0 for command ,RW=0 for write, E=0 to start from low
	GPIO_PORTB_DATA_R=com; //put the com inside the data(portB) store in LCD from D0 to D7
	GPIO_PORTC_DATA_R=E; // change from low to high
	GPIO_PORTC_DATA_R=0;
	if(com<=2) delayMs(2);// if com is clear(1) or return home(2) execution time will be 1.64 ms 
	else delayUs(40); /*any com else execution time will be 40us*/ 
}
void LCD_Data(char data)
{
	GPIO_PORTC_DATA_R=Rs; // set Rs=1 and RW=0 pC5&pC6
	GPIO_PORTB_DATA_R=data; // put the data entered from user in data(portB) in LCD
	GPIO_PORTC_DATA_R|=E;
	delayMs(0);
	GPIO_PORTC_DATA_R=0;
}
void LCD_Init()
{
	SYSCTL_RCGCGPIO_R|=0x06; //set portB for data in LCD and portC for Rs, RW, E
	while((SYSCTL_RCGCGPIO_R & 0x06)!=0x06);
	GPIO_PORTB_DIR_R=0xFF; //data(portB) in LCD set as OUTPUTS
	GPIO_PORTB_DEN_R=0xFF; //data(portB) in LCD set as DIGITAL
	GPIO_PORTC_DIR_R=0xE0; //set pC5(Rs), pC6(RW), pC7(E) as OUTPUTS
	GPIO_PORTC_DEN_R=0xE0; //set pC5(Rs), pC6(RW), pC7(E) as DIGITAL
	LCD_Command(LCD_wakeup); 
	//we can insert delay between LCD_Command
  LCD_Command(LCD_8bitData);
	LCD_Command(LCD_incrementCursor);
	LCD_Command(LCD_clear);
	LCD_Command(LCD_displayOn);
}
unsigned char getKey()
{
	int row,column;
	GPIO_PORTA_DATA_R=0; 
	column=GPIO_PORTA_DATA_R&0xF0; //read from pA4 to pA7 (Columns)
	if(column==0xF0) return 0;// No button is pressed
	while(1) //if a button is pressed (this loop to showwhich row is enabled)
	{column=column|0x0E; if(column!=0xF0){row=0; break;}
		column=column|0x0D; if(column!=0xF0){row=1; break;}
		column=column|0x0B; if(column!=0xF0){row=2; break;}
		column=column|0x07; if(column!=0xF0){row=0; break;}} 
	//check which button is pressed and show the pressed button on the LCD
	if(column==0xE0){ LCD_Data(keyButton[row][0]); return keyButton[row][0];}
	if(column==0xD0){ LCD_Data(keyButton[row][1]); return keyButton[row][1];}
	if(column==0xB0){ LCD_Data(keyButton[row][2]); return keyButton[row][2];}
	if(column==0x70){ LCD_Data(keyButton[row][3]); return keyButton[row][3];} else return 0; 
}
void open_Door() //Initialize portF and connect pA6 to solenoid
{
	SYSCTL_RCGCGPIO_R |=0x20; //set for portF
	while((SYSCTL_RCGCGPIO_R & 0x20)!=0x20){}
  GPIO_PORTF_LOCK_R=0x4C4F434B; GPIO_PORTF_CR_R=0x1F;
	GPIO_PORTF_AFSEL_R=0; GPIO_PORTF_PCTL_R=0;
	GPIO_PORTF_AMSEL_R=0; GPIO_PORTF_DIR_R=0x0E; 
	GPIO_PORTF_DEN_R=0x1F; GPIO_PORTF_PUR_R=0x11;
	GPIO_PORTF_DATA_R=0; SYSCTL_RCGCGPIO_R |=0x01; //set for portA
	while((SYSCTL_RCGCGPIO_R & 0x01)!=0x01){}
  GPIO_PORTA_LOCK_R=0x4C4F434B; GPIO_PORTA_CR_R |=0x04;
	GPIO_PORTA_AFSEL_R|=0; GPIO_PORTA_PCTL_R|=0;
	GPIO_PORTA_AMSEL_R|=0; GPIO_PORTA_DIR_R|=0x04;
	GPIO_PORTA_DEN_R|=0x04; GPIO_PORTA_DATA_R|=0;
}
void UART0_Init()
{
	SYSCTL_RCGCUART_R |=0x01; /*set UART0*/ 
	while((SYSCTL_RCGCUART_R & 0x01)!=0x01);
	SYSCTL_RCGCGPIO_R |=0x01; /*set PORTA 000 001*/
	while((SYSCTL_RCGCGPIO_R & 0x01)!=0x01);
	//Initialization for pA0, pA1(enable them only)
	GPIO_PORTA_LOCK_R=0x4C4F434B; 
	GPIO_PORTA_CR_R |=0x03; //pA0 and pA1
	GPIO_PORTA_AFSEL_R |=0x03; /*pA0 and pA1*/ 
	GPIO_PORTA_PCTL_R =0x11; //pin1 (4bits(4->7)),pin0 (4bits(0->3))
	GPIO_PORTA_DEN_R |=0x03; /*pA0, pA1 digital enable*/ 
	GPIO_PORTA_AMSEL_R &=0;
	//UART0 Initialization
	UART0_CTL_R&=~0x01; /*disable uart0*/
  UART0_IBRD_R=104; //assume that system clock is 9600 so,(16,000,000)/(16*9600)=104.16666666666666
  UART0_FBRD_R=11; //Divisor fraction=integer((0.166667*64)+0.5)=11		
	UART0_LCRH_R|=0x0070;//8-bits(0x30), No-parity, 1-bit stop
  UART0_CC_R=0x0; /*System clock*/ UART0_CTL_R|=0x301; /*enable UART0, TXE, RXE*/
}
char recieveChar()
{
	char R; 
	while((UART0_FR_R&0x10)!=0); // check if RXFE(bit no 4) is not empty so we can recieve another char and read it from the data
		R=UART0_DR_R; 
  return R; 
}
void transmitChar(char T)
{
	while((UART0_FR_R&0x20)!=0); // check if TXFF(bit no 5) is not full so we can send another char and write it in the data
	UART0_DR_R=T;
}
void systemOut(char * c)
{
	while(*c)
		{
		transmitChar(*(c++));
		}
}
int main()
	{
		int i=0; 
		int j=0;
	  char room_status[5]; /*status for each room(5 rooms)*/ 
		char number_room[5]; //assume number of room is 5 rooms
	  char user_Password[4][5]; //the password entered by user 
	  keyPad_Init();
		LCD_Init(); 
		UART0_Init(); 
		open_Door();
	  while(1)
		{
			room_status[i]=recieveChar(); 
			transmitChar(room_status[i]);
			number_room[i]=recieveChar();
			transmitChar(number_room[i]); 
			systemOut(" ");
			switch(room_status[i])
				{ 
					//check the status of each room FREE, OCUPPIED, CLEANING
					case 'F':{ //check if the room is free
						if(number_room[i]=='1') GPIO_PORTF_DATA_R=Red; //Red led turns on
						else break;} break;
					case 'C':{ //check if the room is cleaning
						if(number_room[i]=='1') GPIO_PORTF_DATA_R=Blue; //Blue Led turns on
						else break;} break;
					case 'O':{ //check if the room is ocuppied
						for(j=0;j<4;j++)
						{ //the user insert the password of the room
							user_Password[j][i]=recieveChar(); 
						  transmitChar(user_Password[j][i]);}
						  if(number_room[i]=='1')
								{ // we can open only room number 1 so if room number is 1 so we enter to check the password entered by guest
									if((password[0]==user_Password[0][i])&&(password[1]==user_Password[1][i])&&(password[2]==user_Password[2][i])&&(password[3]==user_Password[3][i])) // check if the user enter the right password 
										GPIO_PORTF_DATA_R=Green; //Green led turns on
									else /*if the guest enter wrong password*/ 
								    GPIO_PORTF_DATA_R=Yellow;} //led 3 and 1 turn on together (Yellow)
							else break; } break;
				  default: 
						GPIO_PORTF_DATA_R=White; //three leds turn on together (White)
			      break; 
				}
		i++;
	systemOut("\n\r");
	}
	}