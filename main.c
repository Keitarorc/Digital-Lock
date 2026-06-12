#include <TM4C123GH6PM.h>

void delayUs(int n);
void sound(void);
void LCD_command(unsigned char command);
void PORTS_init(void);
void LCD_nibble_write(char data, unsigned char control); 
void LCD_init(void);
void LCD_data(char data);
void high_sound(void);
void low_sound(void);
void delayMs(int n);


int password_length = 0;
int index = 0;
char user_input;
char numbers[10] = {'0','1','2','3','4','5','6','7','8','9'};
int test_password = 0;\
char correct_password[4] = "1234";
char user_password[4];
int set_number = 0;
int count = 0;

/* 	maybe add a count variable to keep track of
		how many number the user has inputed because 
		if they reach for and the password is not correct
		the system should reset it and make the buzzer 
		play a sound that they did the wrong password.*/
int main(void) {
	
	// Enabling ports for the LCD 
	SYSCTL->RCGCGPIO |= 0x01;	// Enable clock for GPIO A
	GPIOA->AMSEL &= ~0x3C;      
	GPIOA->DATA &= ~0x3C;
	GPIOA->DIR |= 0x3C;				// output PA2-PA5
	GPIOA->DEN |= (0x3C);  		// digital  
	
	SYSCTL -> RCGCGPIO |= 0x10; //E 
	GPIOE->AMSEL &= ~0x01;       
	GPIOE->DIR |= 0x01; 
	GPIOE->DEN |= 0x01; 
	GPIOE->DATA |= 0x01;
	
	SYSCTL->RCGCGPIO |= 0x04;	// enable clock to GPIOC 
	GPIOC->AMSEL &= ~0x50;    // disable analog function 
	GPIOC->DIR |= 0x50;       // set PC4 as output pin 
	GPIOC->DEN |= 0x50;       // set PC4 as digital pin
	GPIOC->DATA &= ~0x40;
	
	// Enabling port for the pushbuttons
	SYSCTL->RCGCGPIO |= 0x08;	// Enable clock for GPIO D
	GPIOD->DIR &= ~(0x0F); 		// input PD0-PD3
	GPIOD->DEN |= (0x0F);  		// digital
	
	// Enabling and setting up Timer
	SYSCTL->RCGCTIMER |= 1;		// Enable clock for TIMER0	
	TIMER0->CTL = 0; 					// Disable TIMER0
	TIMER0->CFG = 0x04; 			// Choosing 16 bit timer
	TIMER0->TAMR = 0x02; 			// Periodic mode
	TIMER0->TAILR = 50 - 1; 	// Loading register with 50
	TIMER0->ICR = 0x1; 				// Clearing Interrupt
	TIMER0->CTL |= 0x01;			// Enable TIMER0
	
	// Enabling port for the 4 LEDs on the Tiva board 
	SYSCTL->RCGCGPIO |= 0x02;	// Enable clock for GPIOB
	GPIOB->DIR |= 0x0F;				//
	GPIOB->DEN |= 0x0F;
	
	/* configure PORTD3-0 for falling edge trigger interrupt */ 
	GPIOD->IS &= ~(0x0F);  /* make bit 3-0 edge sensitive */ 
	GPIOD->IBE &= ~(0x0F); /* trigger is controlled by IEV */ 
	GPIOD->IEV &= ~(0x0F); /* falling edge trigger */ 
	
	GPIOD->ICR |= (0x0F);  /* clear any prior interrupt */ 
	GPIOD->IM |= (0x0F);   /* unmask interrupt */ 
	
	NVIC->IPR[3] = 1 << 5;    /* set interrupt priority to 1 */ 
	NVIC->ISER[0] |= (0x08); /* enable IRQ3 */
	
	// Setup and diplay password to the LCD
	LCD_init();
	
	LCD_data('P'); 
	LCD_data('A'); 
	LCD_data('S'); 
	LCD_data('S'); 
	LCD_data('W'); 
	LCD_data('O'); 
	LCD_data('R'); 
	LCD_data('D'); 
	
	LCD_command(0xC0);
	
	// setting up PWM for the servo motor
	
	SYSCTL->RCGCPWM |= 0x02;        //(1) enable clock to PWM1 
	SYSCTL->RCGCGPIO |= 0x20;       //(2) enable clock to GPIOF 
	SYSCTL->RCC |=  0x00100000;     //(3) enable clock divider for PWM 
	SYSCTL->RCC &= ~0x000E0000;     //(4) clear clock divide 
	SYSCTL->RCC |=  0x00060000;     //(5) set clock divide to 16 (50 MHz/16 = 3.125 MHz) 
	
	delayUs(1000);                  //(6) wait for PWM clock to stabilize 
	
	PWM1->_3_CTL = 0;               //(7) disable PWM1_3 during configuration 
	PWM1->_3_GENB = 0x0000080C;     //(8) output high when load and low when match 
	PWM1->_3_LOAD = 62500 - 1;      //(9) 50 Hz  
	PWM1->_3_CMPB = 56250 - 1;      //(10) 5% DUTY CYCLE: 59375, 10% DUTY CYCLE: 56250 
	PWM1->_3_CTL = 1;               //(11) enable PWM1_3, set counter mode to count down 
	PWM1->ENABLE |= 0x80;           //(12) enable PWM1 
	
	GPIOF->DIR |= 0x08;             //(13) set PORTF 3 pins as output pin 
	GPIOF->DEN |= 0x08;             //(14) set PORTF 3 pins as digital pins 
	GPIOF->AFSEL |= 0x08;           //(15) enable alternate function 
	GPIOF->PCTL &= ~0x0000F000;     //(16) clear PORTF 3 alternate function 
	GPIOF->PCTL |= 0x00005000;      //(17) set PORTF 3 alternate function to PWM
	
	while(1) {
		if (set_number == 1){
			user_password[password_length] = user_input;
			password_length++;
			set_number = 0;
		}
		// Checking to see if inputed password 
		if (test_password == 1) {
			for (int i = 0; i < 4; i++) {
				if (user_password[i] == correct_password[i]){
					count++;
					if (count == 4){
						LCD_command(0x01);
		
						LCD_data('U'); 
						LCD_data('N'); 
						LCD_data('L'); 
						LCD_data('O'); 
						LCD_data('C'); 
						LCD_data('K'); 
						LCD_data('E'); 
						LCD_data('D'); 
					
						LCD_command(0xC0);
						
						sound();
						delayUs(20);
						sound();
						delayUs(20);
						sound();
						delayUs(20);
						high_sound();
						for (int pw = 56250; pw < 59375; pw += 50) { 
							PWM1->_3_CMPB = pw; 
							delayMs(5); 
						}   
						delayMs(1000);
						
						for (int pw = 59375; pw > 56250; pw -= 50) { 
							PWM1->_3_CMPB = pw; 
							delayMs(5); 
						}
						
						//clearing the correct password from memory
						for (int i = 0; i < 4; i++) {
							user_password[i] = ' ';
						}
						
						LCD_command(0x01);
						
						LCD_data('P'); 
						LCD_data('A'); 
						LCD_data('S'); 
						LCD_data('S'); 
						LCD_data('W'); 
						LCD_data('O'); 
						LCD_data('R'); 
						LCD_data('D'); 
	
						LCD_command(0xC0);
					}
				}
				else {
					LCD_command(0x01);
		
					LCD_data('W'); 
					LCD_data('R'); 
					LCD_data('O'); 
					LCD_data('N'); 
					LCD_data('G'); 
					LCD_data(' '); 
					LCD_data('P'); 
					LCD_data('A'); 
					LCD_data('S'); 
					LCD_data('S'); 
					LCD_data('W'); 
					LCD_data('O'); 
					LCD_data('R'); 
					LCD_data('D'); 
						
					LCD_command(0xC0);
					low_sound();
					delayUs(20);
					low_sound();
					delayUs(20);
					low_sound();
					delayUs(20);
					delayUs(300000);
					
					LCD_command(0x01);
					
					LCD_data('P'); 
					LCD_data('A'); 
					LCD_data('S'); 
					LCD_data('S'); 
					LCD_data('W'); 
					LCD_data('O'); 
					LCD_data('R'); 
					LCD_data('D'); 
	
					LCD_command(0xC0);
					
					break;
				}
			}
			password_length = 0;
			test_password = 0;
			count = 0;
		}
	}
}

void GPIOD_Handler(void) { 
	
	if (GPIOD->MIS & 0x8) {  //SW2 
		// Choosing from row 1
		// Rotate thorught the numbers
		index = (index + 1) % 10;
		user_input = numbers[index];
		GPIOB->DATA = index;
		GPIOD->ICR |= 0x8; /* clear the interrupt flag */ 
	} 
	else if (GPIOD->MIS & 0x04) { //SW3 
		// Choosing from row 2
		// Enter the number to print to the LCD
		LCD_data(user_input);
		set_number = 1;
		index = 0;
		GPIOD->ICR |= 0x04; /* clear the interrupt flag */ 
	}  
	else if (GPIOD->MIS & 0x02) { //SW4 
		// Choosing from row 3
		// Clear password
		LCD_command(0x01);
		
		LCD_data('P'); 
		LCD_data('A'); 
		LCD_data('S'); 
		LCD_data('S'); 
		LCD_data('W'); 
		LCD_data('O'); 
		LCD_data('R'); 
		LCD_data('D'); 
	
		LCD_command(0xC0);
		password_length = 0;
		for (int i = 0; i < 4; i++) {
			user_password[i] = ' ';
		}
		GPIOD->ICR |= 0x02; /* clear the interrupt flag */ 
	} 
	else if (GPIOD->MIS & 0x01) { //SW5
		// Choosing from row 4
		// Enter password to get checked to see if it is correct
		test_password = 1;
		GPIOD->ICR |= 0x01; 
	}
	//sound();
}	

void delayUs(int ttime) {//microsecond delay for 50 MHz clock  
	int i; 
	for (i = 0; i < ttime; i++) {
		while ((TIMER0->RIS & 0x1) == 0); 
		TIMER0->ICR = 0x1;
	}		
}

void sound(void) {
	for(int i = 0; i < 200; i++){
	GPIOC->DATA |= 0x10;       //turn on PC4 
	delayUs(300);              //Half Period 
	GPIOC->DATA &= ~0x10;      //turn off PC4 
	delayUs(300);   
	}		//Half period 
}

void high_sound(void) {
	for(int i = 0; i < 800; i++){
	GPIOC->DATA |= 0x10;       //turn on PC4 
	delayUs(150);              //Half Period 
	GPIOC->DATA &= ~0x10;      //turn off PC4 
	delayUs(150);   
	}		//Half period 
}

void low_sound(void) {
	for(int i = 0; i < 20; i++){
	GPIOC->DATA |= 0x10;       //turn on PC4 
	delayUs(1500);              //Half Period 
	GPIOC->DATA &= ~0x10;      //turn off PC4 
	delayUs(1500);   
	}		//Half period 
}

void PORTS_init(void) {
	/*SYSCTL -> RCGCGPIO |= 0x01; //A 
	SYSCTL -> RCGCGPIO |= 0x10; //E 
	SYSCTL -> RCGCGPIO |= 0x04; //C
	
	GPIOA->AMSEL &= ~0x3C;      
	GPIOA->DATA &= ~0x3C; 
	GPIOA->DIR |= 0x3C;          
	GPIOA->DEN |= 0x3C; 

	GPIOE->AMSEL &= ~0x01;       
	GPIOE->DIR |= 0x01; 
	GPIOE->DEN |= 0x01; 
	GPIOE->DATA |= 0x01;
	
	GPIOC->AMSEL &= ~0x40;       
	GPIOC->DIR |= 0x40;          
	GPIOC->DEN |= 0x40;          
	GPIOC->DATA &= ~0x40;*/ 
}

void LCD_nibble_write(char data, unsigned char control) { 
	GPIOA->DIR |= 0x3C; //setting PA2-PA5 as outputs (interface to LCD)           
	GPIOA->DATA &= ~0x3C; //clearing the line 
	GPIOA->DATA |= (data & 0xF0) >> 2; //extracting the upper nibble 
	/* set RS bit */ 
	if (control & 1) {          //control & RS 
		GPIOE->DATA |= 1;  //check if LCD data is data 
	}
	else {  
		GPIOE->DATA &= ~1;  //check if LCD data is command 
	}

	/* sending a high to low transition pulse on LCD enable pin (PC6) */ 
	GPIOC->DATA |= 1 << 6;   
	delayUs(0); 
	GPIOC->DATA &= ~(1 << 6); 
	GPIOA->DIR &= ~0x3C; //clearing the line
}	

void LCD_command(unsigned char command) { 
	LCD_nibble_write(command & 0xF0, 0); //upper nibble 
	LCD_nibble_write(command << 4, 0);   //lower nibble 
	if (command < 3) { 
		delayUs(2000);   //maximum delay of 2 ms for the first two instructions         
	}
	else { 
		delayUs(1000); //maximum delay of 1 ms for all other instructions
	}
} 

void LCD_init(void) { 
	PORTS_init(); 
	//initialization sequence 
	delayUs(20000);                 
	LCD_nibble_write(0x30, 0); 
	delayUs(5000); 
	LCD_nibble_write(0x30, 0); 
	delayUs(1000); 
	LCD_nibble_write(0x30, 0); 
	delayUs(1000); 
	
	//configuration 
	LCD_nibble_write(0x20, 0);  // use 4-bit data mode 
	delayUs(1000); 
	LCD_command(0x28);          // set 4-bit data, 2-line, 5x7 font 
	LCD_command(0x06);          // move cursor right 
	LCD_command(0x01);          // clear screen, move cursor to home 
	LCD_command(0x0F);    			// turn on LCD
}	

void LCD_data(char data) { 
	LCD_nibble_write(data & 0xF0, 1); //upper nibble      
	LCD_nibble_write(data << 4, 1);   //lower nibble 
	delayUs(1000); 
} 

/* delay n milliseconds (50 MHz CPU clock) */ 
void delayMs(int n) { 
	int i, j; 
	for(i = 0 ; i< n; i++) 
		for(j = 0; j < 6265; j++) 
			{}  /* do nothing for 1 ms */ 
}


