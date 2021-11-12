#include<stdio.h>

//register address
#define RCC_BASE_ADDR 							0x40021000UL
#define RCC_AHB2_ENR_OFFSET						0x4CUL
#define RCC_AHB2_ENR_ADDR  						(RCC_BASE_ADDR + RCC_AHB2_ENR_OFFSET)
#define RCC_APB2_ENR_OFFSET						0x60UL
#define RCC_APB2_ENR_ADDR  						(RCC_BASE_ADDR + RCC_APB2_ENR_OFFSET)


#define GPIOA_BASE_ADDR							0x48000000UL 
#define GPIOA_MODE_REG 							(GPIOA_BASE_ADDR + 0x00)
#define GPIOA_SPEED_REG 						(GPIOA_BASE_ADDR + 0x08)
#define GPIOA_PUPD_REG 							(GPIOA_BASE_ADDR + 0x0C)
#define GPIOA_OPTYPE_REG 						(GPIOA_BASE_ADDR + 0x04)
#define GPIOA_OUTPUTDATA_REG 						(GPIOA_BASE_ADDR + 0x14)

#define	APB2PERIPH_BASE_ADDR						0x40010000UL
#define	SYSCFG_BASE_ADDR						APB2PERIPH_BASE_ADDR
#define	SYSCFG_EXTICR1_REG						(SYSCFG_BASE_ADDR + 0x0C)
#define	EXTI_BASE_ADDR							(APB2PERIPH_BASE_ADDR + 0x0400)
#define	EXTI_FTSR1_REG							(EXTI_BASE_ADDR + 0x0C)
#define	EXTI_RTSR1_REG							(EXTI_BASE_ADDR + 0x08)
#define	EXTI_IMR1_REG							(EXTI_BASE_ADDR + 0x00)
#define	EXTI_PR1_REG							(EXTI_BASE_ADDR + 0x14)

#define	IRQ_NO_EXTI9_5							23
#define NVIC_ISER0							((uint32_t*) 0xE000E100UL)

#define BuiltInLED							5
#define ExtButton							7


static int leds[6] = {6,12,11,9,8,10}; // array for LED pin numbers 
static _Bool dir = 1; // variable for LED direction


void delay(void){
	for(uint32_t i=0; i < 50000 ; i++);
}

int main(void){
	
	// Enable RCC for GPIOA
	uint32_t *pRccAhb2Enr = (uint32_t*) (RCC_AHB2_ENR_ADDR);
	*pRccAhb2Enr |= (1 << 0);

	// GPIOA port register pointers
	uint32_t *GPIOA_MDOE = (uint32_t*) (GPIOA_MODE_REG);
	uint32_t *GPIOA_SPEED = (uint32_t*) (GPIOA_SPEED_REG );
	uint32_t *GPIOA_PUPD = (uint32_t*) (GPIOA_PUPD_REG);
	uint32_t *GPIOA_OPTYPE = (uint32_t*) (GPIOA_OPTYPE_REG );
	uint32_t *GPIOA_OUTPUTDATA = (uint32_t*) (GPIOA_OUTPUTDATA_REG );

	// set registers for each LED
	for(int i=0; i<6 ; i++){
		
		// Clean and set GPIOA mode register to output
		*GPIOA_MDOE &= ~( 3 << ( leds[i] *2) );
		*GPIOA_MDOE |= ( 1 << ( leds[i] *2) ) ;

		// Clean and set GPIOA speed register to high
		*GPIOA_SPEED &= ~( 3 << ( leds[i] *2) );
		*GPIOA_SPEED |= ( 2 << ( leds[i] *2) ) ;

		// Clean and set GPIOA pull up/ pull down register to pull up
		*GPIOA_PUPD &= ~( 3 << ( leds[i] *2) );
		*GPIOA_PUPD |= ( 1 << ( leds[i] *2) ) ;

		// Clean and set GPIOA output type register to push-pull
		*GPIOA_OPTYPE &= ~( 1 << leds[i] );
	}


	/* 
	Button and interrupt configuration
	*/

	//Clean and set GPIOA mode register to input
	*GPIOA_MDOE &= ~( 3 << ( ExtButton *2) );
	
	//Set falling trigger selection register
	uint32_t *EXTI_FTSR1 = (uint32_t*) (EXTI_FTSR1_REG);
	*EXTI_FTSR1 |= (1 << ExtButton);

	//Clear rising trigger selection register
	uint32_t *EXTI_RTSR1 = (uint32_t*) (EXTI_RTSR1_REG);
	*EXTI_RTSR1 &= ~(1 << ExtButton);

	//Enable clock for system configuration controller
	uint32_t *pRccApb2Enr = (uint32_t*) (RCC_APB2_ENR_ADDR);
	*pRccApb2Enr |= (1 << 0);

	//Configure GPIO port selection - for GPIO port A isn't needed, for other ports must be set
	//uint32_t *SYSCFG_EXTICR1 = (uint32_t*) (EXTI_RTSR1_REG);
	//*SYSCFG_EXTICR1 |= (0 << ExtButton %4);

	//Enable EXTI interrupt delivery using IMR - interrupt mask register
	uint32_t *EXTI_IMR1 = (uint32_t*) (EXTI_IMR1_REG);
	*EXTI_IMR1 |= (1 << ExtButton);

	// Clean and set GPIOA speed register to high for button
	*GPIOA_SPEED &= ~( 3 << ( ExtButton *2) );
	*GPIOA_SPEED |= ( 2 << ( ExtButton *2) ) ;

	// Clean and set GPIOA pull up/ pull down register to no pull up/ pull down
	*GPIOA_PUPD &= ~( 3 << ( ExtButton *2) );
	*GPIOA_PUPD |= ( 0 << ( ExtButton *2) ) ;

	//IRQ setup
	*NVIC_ISER0 |= (1 << IRQ_NO_EXTI9_5) ;

	int i = 0; // variable for move through LED array

	while(1){

		// turn specific LED on, wait and turn it off
		*GPIOA_OUTPUTDATA ^= ( 1 << leds[i]) ;
		delay();
		*GPIOA_OUTPUTDATA ^= ( 1 << leds[i]) ;

		// make changes to direction in witch LEDs turn on and off
		if (dir == 0)
			i++;
		else if (dir == 1)
			i--;

		// ensure that only defined LEDs from an array are used
		if (i < 0)
			i = 6;
		else if (i > 6)
			i = 0;
	}
}

void EXTI9_5_IRQHandler(void){

	// Clean pending interrupt flag register
	uint32_t *EXTI_PR1 = (uint32_t*) (EXTI_PR1_REG);
	if(*EXTI_PR1 & ( 1 << ExtButton))
		*EXTI_PR1 |= ( 1 << ExtButton);
	
	//change variable used for LED direction
	dir ^= 1;
	
}

