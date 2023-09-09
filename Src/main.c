/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Shady Taha
 * @brief          : Main program body
 ******************************************************************************
 */



/****************************INCLUDED FILES*****************************************/
#include"CortexM3/CORTEXM3_Interface.h"
#include "MCAL_Drivers/RCC/STM32F103xx_HAL_RCC.h"
#include "MCAL_Drivers/GPIO/STM32F103xx_HAL_GPIO_Interface.h"
#include "MCAL_Drivers/UART/STM32F103xx_HAL_UART_Interface.h"
#include "MCAL_Drivers/FLASH_DRIVER/STM32F103xx_HAL_FPEC_Interface.h"
#include "PARSER/PARSER.h"
#include "HAL_Drivers/WIFI/WIFI_Initerface.h"
/***********************************************************************************/


/*object choose HSI as clock source*/
RCC_OscInitTypeDef RCC_Init={RCC_OSCILLATORTYPE_HSI,RCC_HSE_OFF,RCC_LSE_OFF,RCC_HSI_ON,RCC_LSI_OFF,HSI};

/*object select the Pre-scaler values for busses*/
RCC_ClkInitTypeDef RCC_CLK_Init={AHB_divided_by_1, APB_divided_by_1,APB_divided_by_1};

/*object to enable RCC for UART*/
Enable_Clock_values Clock_values1={RCC_APB2ENR,UART1EN};

/*object to enable RCC for GPIO*/
Enable_Clock_values Clock_values2={RCC_APB2ENR,IOPAEN};

/*object to enable RCC for Flash Driver*/
Enable_Clock_values Clock_values3={RCC_AHBENR,FPECEN};


/*object to set the sys-tick configurations */
SYSTICK_Init Sys_Tick_init={CLKSOURCE_external_clock,15000000};

/*Enable TX pin of UART*/
extern PIN_State State;

/*Enable RX pin of UART*/
extern PIN_State State2;

/*flag set when i receive SEND FAILED*/
extern u8 SEND_FAILED_Flag;

/*************************WEBSITE URLS****************************************/

/*URL of Update.txt to check if there is a file uploaded to web-site or not*/
u8 Update_Check_URL[]="http://otaupdatesolutions.orgfree.com/Update.txt";

/*URL of Receive_record.php to break down the record*/
u8 Receive_RecordPHP[]="http://otaupdatesolutions.orgfree.com/Receive_record.php?response=ok";


/*URL of Record i will receive object to enable RCC for Flash Driver*/
u8 Record_File[]="http://otaupdatesolutions.orgfree.com/Record_line.txt";

/*****************************************************************************/


/******** Macros to check if there is an update required or not************/
#define NOT_OK_Update 	'0'
#define OK_Update 		'1'


/******** Macros to check if there is an update required or not************/
#define HEX_File_End 	'1'

/*********************Records digits************************************/
#define Record_Type 	8
/***********************************************************************/


typedef void(*function_t)(void);
function_t add_to_call=0;
void func(void)
{
	/*Address of SCB_VTOR Register*/
#define SCB_VTOR  *((volatile u32*)0xE000ED08)
	SCB_VTOR=0x08001C00;
	add_to_call=*(function_t*)(0x8001C04);
	add_to_call();
}

u8 NO_RECORD_RECEIVED_FLAG=0;
extern u8 HTTP_Request_FLAG;
u8 URL_Request_FLAG=0
int main(void)
{
	/*array to receive record in*/
	u8 Receive_arr[100]={0};

	/*array to insert in it record*/
	u8 Record_arr[100]={0};

	/*initialize the RCC*/
	HAL_RCC_OscConfig(&RCC_Init);

	/*select the Pre-scaler values for busses*/
	HAL_RCC_ClockConfig(&RCC_CLK_Init);

	/*enable clock to UART peripheral*/
	HAL_RCC_ClockEnable(&Clock_values1);

	/*enable clock to GPIO peripheral*/
	HAL_RCC_ClockEnable(&Clock_values2);

	/*enable clock to Flash Driver peripheral*/
	HAL_RCC_ClockEnable(&Clock_values3);

	/*initialize UART peripheral*/
	MUSART1_VoidInit();

	/*select PIN A9 as TX */
	GPIO_VoidSetPinDirection(&State);

	/*select PIN A10 as RX*/
	GPIO_VoidSetPinDirection(&State2);

	/*ESP8266 initialization*/
	ESP_VoidInit();

	/*ESP8266 connect to WIFI*/
	ESP_VoidWifiConnect();

	/*ESP8266 connect to server*/
	ESP_VoidServerConnect();

	/*set Update  URL flag */
	URL_Request_FLAG=1;

	/*send HTTP request to the server*/
	ESP_VoidServerRequest(Receive_arr,(u8*)"54",(u8*)Update_Check_URL);

	/*clear Update  URL flag */
	URL_Request_FLAG=0;

	/*initialize the sys-tick timer*/
	Systick_Init(&Sys_Tick_init);

	/*activate sys-tick interrupt and set time to be 15 seconds*/
	Systick_SingleInterval(15000000,func);


	while(1)
	{

		/*check if there are no any updates go to APP*/
		if(Receive_arr[40] == NOT_OK_Update)
		{

		}else if(Receive_arr[40] == OK_Update)
		{
			/*if there is an update disable the sys-tick timer*/
			Systick_DEInit();

			/*flush the receive array*/
			ESP_VoidArrayFlush(Receive_arr);

			/*delete the Application Area*/
			FPEC_voidEraseAPPArea();

			/*if record type is 1 this means code reached the end of the hex file*/
			while(Record_arr[Record_Type]!= HEX_File_End)
			{
				/*flush both arrays*/
				ESP_VoidArrayFlush(Receive_arr);
				ESP_VoidArrayFlush(Record_arr);

				/*ESP8266 connect to server*/
				ESP_VoidServerConnect();

				/*send HTTP request to the server*/
				ESP_VoidServerRequest(Receive_arr,(u8*)"74",(u8*)Receive_RecordPHP);

				/*flush the receive array*/
				ESP_VoidArrayFlush(Receive_arr);

				/*ESP8266 connect to server*/
				ESP_VoidServerConnect();
				/*set A Flag to increase the count of the receive in UART_Receive*/
				HTTP_Request_FLAG=1;

				/*send HTTP request to server*/
				ESP_VoidServerRequest(Receive_arr,(u8*)"59",(u8*)Record_File);

				/*set A Flag to increase the count of the receive in UART_Receive*/
				HTTP_Request_FLAG=0;
				/*produce pure Record file*/
				ConvertRecieveToRecord(Receive_arr,Record_arr);

				/*if to see if no record is send or anything send not a record  & : is 58 in ascii*/
				if(Record_arr[0] != ':')
				{
					NO_RECORD_RECEIVED_FLAG=1;
					while(NO_RECORD_RECEIVED_FLAG==1)
					{
						NO_RECORD_RECEIVED_FLAG=0;
						/*ESP8266 connect to server*/
						ESP_VoidServerConnect();
						/*set A Flag to increase the count of the receive in UART_Receive*/
						HTTP_Request_FLAG=1;

						/*send HTTP request to server*/
						ESP_VoidServerRequest(Receive_arr,(u8*)"59",(u8*)Record_File);

						/*set A Flag to increase the count of the receive in UART_Receive*/
						HTTP_Request_FLAG=0;

						/*produce pure Record file*/
						ConvertRecieveToRecord(Receive_arr,Record_arr);

						/*if still first in Reacord_arr not equal : set the NO_RECORD_RECEIVED_FLAG to be 1*/
						if(Record_arr[0] != ':')
						{
							NO_RECORD_RECEIVED_FLAG=1;
						}
					}

				}
				/*send record array to parser*/
				Parser_voidParseRecord(Record_arr);

		}

		/*activate sys-tick interrupt and set time to be 15 seconds*/
		Systick_SingleInterval(15000000,func);

	}

		//Receive_arr[40] = NOT_OK_Update;
	/*ESP8266 connect to server*/
	ESP_VoidServerConnect();

	/*send HTTP request to the server*/
	ESP_VoidServerRequest(Receive_arr,(u8*)"54",(u8*)Update_Check_URL);
}
}

