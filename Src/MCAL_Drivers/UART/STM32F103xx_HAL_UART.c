/*
 * STM32F103xx_HAL_UART.c
 *
 *  Created on: Jul 29, 2023
 *      Author: shady
 */
#include"Library/STD_TYPES.h"
#include"Library/BIT_MATH.h"
#include"MCAL_Drivers/Common/STM32F103xx_HAL_Def.h"
#include"MCAL_Drivers/UART/STM32F103xx_HAL_UART_Private.h"
#include "MCAL_Drivers/UART/STM32F103xx_HAL_UART_Cfg.h"

extern u8 WIFI_Connect_FLAG;
extern u8 HTTP_Request_FLAG;
extern u8 URL_Request_FLAG;
void MUSART1_VoidInit(void)
{
	/*For Baud Rate 115200*/
	UART->UART_BRR=BaudRate;
	/*Enable RX*/
	SET_BIT(UART->UART_CR1,RX_Enable);
	/*Enable TX*/
	SET_BIT(UART->UART_CR1,TX_Enable);
	/*Enable UART*/
	SET_BIT(UART->UART_CR1,UART_Enable);
	/*Clear status register*/
	UART->UART_SR=0;
	UART->UART_DR=0;
}
void MUSART1_VoidTransmit(u8 num)
{
	while(!GET_BIT((UART->UART_SR),TC))
	{

	}
	UART->UART_DR=num;
}


void MUSART1_VoidTransmitArray(u8 *arr)
{
	u8 i=0;

	while(arr[i] != '\0')
	{
		MUSART1_VoidTransmit(arr[i]);
		i++;
	}
}

u8 MUSART1_VoidReceive(void)
{
	u8 Local_Temp=0;
	u32 counter=0;
	while(!GET_BIT((UART->UART_SR),RXNE))
	{
		counter++;
		if(counter == 10000 && WIFI_Connect_FLAG==0 && URL_Request_FLAG==0 && HTTP_Request_FLAG==0)
		{
			Local_Temp=255;
			break;
		}else if(counter == 5000000 && WIFI_Connect_FLAG==1)
		{
			Local_Temp=255;
			break;
		}else if(counter == 200000 && HTTP_Request_FLAG==1)
		{
			Local_Temp=255;
			break;
		}else if(counter == 300000 && URL_Request_FLAG==1)
		{
			Local_Temp=255;
			break;
		}
	}
	if(Local_Temp!=255)
	{
		Local_Temp= ((UART->UART_DR)&0xFF);
	}
	return Local_Temp;
}



UART_DATA_RECIEVE MUSART1_EnumReceiveBuffer(u8 *arr)
{
	UART_DATA_RECIEVE state=DATA_REC;
	if(GET_BIT((UART->UART_SR),RXNE)==1)
	{
	*arr=(UART->UART_DR);
	}else
	{
		state=NO_DATA_REC;
	}
	return state;
}
