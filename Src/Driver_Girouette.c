#include "Driver_Girouette.h"

MyTimer_Struct_TypeDef * myTimerGirouette;

void Init_Girouette(void)
{
	MyGPIO_Struct_TypeDef gpio_0;
	MyGPIO_Struct_TypeDef gpio_1;
	MyGPIO_Struct_TypeDef gpio_2;
	
	// configuration du timer
	myTimerGirouette = malloc(sizeof(MyTimer_Struct_TypeDef));
	myTimerGirouette->Timer = TIM2; // On utilise le Timer 2
	myTimerGirouette->ARR = 718;
	myTimerGirouette->PSC = 1; // valeur par d�faut du prescaler (� verifier)
	
	// initialisation du timer
	MyTimer_Base_Init(myTimerGirouette);
	
	// initialisation des GPIOs
	gpio_0.GPIO_Conf = In_PullUp;
	gpio_1.GPIO_Conf = In_PullUp;
	gpio_2.GPIO_Conf = In_PullUp;
	gpio_0.GPIO = GPIOA;
	gpio_1.GPIO = GPIOA;
	gpio_2.GPIO = GPIOA;
	gpio_0.GPIO_Pin = 0;
	gpio_1.GPIO_Pin = 1;
	gpio_2.GPIO_Pin = 4;
	
	MyGPIO_Init(&gpio_0);
	MyGPIO_Init(&gpio_1);
	MyGPIO_Init(&gpio_2);
	
	// on selectionne les deux entrees
	// entr�e 1
	myTimerGirouette->Timer->CCMR1 &= ~TIM_CCMR1_CC1S_1;
	myTimerGirouette->Timer->CCMR1 |= TIM_CCMR1_CC1S_0;
	
	// entr�e
	myTimerGirouette->Timer->CCMR1 &= ~TIM_CCMR1_CC2S_1;
	myTimerGirouette->Timer->CCMR1 |= TIM_CCMR1_CC2S_0;
	
	// On veut compter les fronts sur deux entr�es, donc SMS = 011
	myTimerGirouette->Timer->SMCR &= ~TIM_SMCR_SMS_2;
	myTimerGirouette->Timer->SMCR |= TIM_SMCR_SMS_1;
	myTimerGirouette->Timer->SMCR |= TIM_SMCR_SMS_0;
	
	// polarit� et "input capture" par d�faut pour les deux entr�es
	// entr�e 1
	myTimerGirouette->Timer->CCER &= ~TIM_CCER_CC1P;
	myTimerGirouette->Timer->CCER &= ~TIM_CCER_CC1NP;
	myTimerGirouette->Timer->CCMR1 &= ~TIM_CCMR1_IC1F;
	
	// entr�e 2
	myTimerGirouette->Timer->CCER &= ~TIM_CCER_CC2P;
	myTimerGirouette->Timer->CCER &= ~TIM_CCER_CC2NP;
	myTimerGirouette->Timer->CCMR1 &= ~TIM_CCMR1_IC2F;
	
	// initialisation de l'interruption lors de la d�tection du zero
	// activation de la AFIO clock
	//RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN ;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	// on configure la pin PA2 comme Alternate Function Input Output
	AFIO->EXTICR[0] |= AFIO_EXTICR2_EXTI4_PA;
	
	// interrupt mask register
	EXTI->IMR |= EXTI_IMR_MR4;
	
	// rising trigger selection register
	EXTI->RTSR |= EXTI_RTSR_TR4;
	EXTI->FTSR &= ~EXTI_FTSR_TR4;
	
	// configuration de l'interruption
	NVIC_SetPriority(EXTI4_IRQn, 1);
	NVIC_EnableIRQ(EXTI4_IRQn);
}

void Start_Mesure_Girouette(void)
{
	// activation du compteur
	myTimerGirouette->Timer->CR1 |= TIM_CR1_CEN;
	MyTimer_Base_Start(myTimerGirouette->Timer);
}	

void Stop_Mesure_Girouette(void)
{
	// d�sactivation du compteur
	myTimerGirouette->Timer->CR1 &= (~TIM_CR1_CEN);
	MyTimer_Base_Stop(myTimerGirouette->Timer);
}

int Get_Angle(void)
{
	return (int) myTimerGirouette->Timer->CNT;
}

void EXTI4_IRQHandler(void)
{
	// reset du flag
	EXTI->PR |= 1;
	// handler
	myTimerGirouette->Timer->CNT = 0;
}
