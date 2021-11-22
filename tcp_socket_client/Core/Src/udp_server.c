#include "main.h"
#include "lwip.h"
#include "sockets.h"
#include "cmsis_os.h"
#include <string.h>

#ifndef UDP_PORTNUM
#define UDP_PORTNUM 5678UL
#endif


#if (USE_TCP_SERVER_PRINTF == 1)
#include <stdio.h>
#define TCP_SERVER_PRINTF(...) do { printf("[udp_server.c: %s: %d]: ",__func__, __LINE__);printf(__VA_ARGS__); } while (0)
#else
#define TCP_SERVER_PRINTF(...)
#endif

static struct sockaddr_in serv_addr, client_addr;

static int socket_fd;
static uint16_t nport;


//osMutexDef(printf_mutex);
//osMutexId(printf_mutex_id);

//#define PRINTF_MUTEX_LOCK() 	osMutexWait (printf_mutex_id, osWaitForever)
//#define PRINTF_MUTEX_UNLOCK()	osMutexRelease(printf_mutex_id)

typedef enum
{
	LED_TOGGLE,
	LED_ON,
	LED_OFF,
	LED_NO_ACTION,
} LED_action_t;

static int udpServerInit(void)
{
	socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_fd == -1) {
		TCP_SERVER_PRINTF("socket() error\n");
		return -1;
	}

	nport = UDP_PORTNUM;
	nport = htons((uint16_t)nport);

	bzero(&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = nport;

	if(bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))==-1) {
		TCP_SERVER_PRINTF("bind() error\n");
		close(socket_fd);
		return -1;
	}




	return 0;
}

void StartUdpServerTask(void const * argument)
{


	osDelay(5000);// wait 5 sec to init lwip stack

	if(udpServerInit() < 0) {
		TCP_SERVER_PRINTF("udpSocketServerInit() error\n");
		return;
	}

	int addr_len;
	int nbytes;
	char buffer[80];
	for(;;)
	{
		 bzero(&client_addr, sizeof(client_addr));
		 bzero(buffer,sizeof(buffer));
		 addr_len = sizeof(client_addr);
		 if((nbytes = recvfrom(socket_fd,buffer,sizeof(buffer),0,(struct sockaddr *)&client_addr,(socklen_t *)&addr_len))>0)
		 {
			//parsing
			TCP_SERVER_PRINTF("data string : %s\n", buffer);

			unsigned int led_number = 0;
			unsigned int led_arr_index = 0;
			unsigned int led_flag= 0;

			LED_action_t LED_action;
			Led_TypeDef led[4] = {LED3, LED4, LED5, LED6};
			char *token = strtok(buffer, " \n");

			if (sscanf(token, "led%u", &led_number) > 0 )
			{
				TCP_SERVER_PRINTF("led number: %u\n", led_number);

				if (led_number >= 3 && led_number <= 6)
				{
					led_flag = 1;
					led_arr_index = led_number - 3;//number in array
				}

			}
			token = strtok(NULL, " \n");
//			TCP_SERVER_PRINTF("led action: %s\n", token);
//			TCP_SERVER_PRINTF("token len %d\n", strlen(token));
			if(token == NULL)
			{
				LED_action = LED_NO_ACTION;
			}
			else if (strncmp(token, "toggle", strlen(token)) == 0 && strlen(token) == strlen("toggle"))
			{
				LED_action = LED_TOGGLE;
			}
			else if (strncmp(token, "on", strlen(token)) == 0 && strlen(token) == strlen("on"))
			{
				LED_action = LED_ON;
			}
			else if (strncmp(token, "off", strlen(token)) == 0 && strlen(token) == strlen("off"))
			{
				LED_action = LED_OFF;
			}
			else
			{
				LED_action = LED_NO_ACTION;
			}

			if(led_flag == 1 && LED_action != LED_NO_ACTION)
			{
				switch (LED_action)
				{
					case LED_TOGGLE:
						BSP_LED_Toggle(led[led_arr_index]);
						break;
					case LED_ON:
						BSP_LED_On(led[led_arr_index]);
						break;
					case LED_OFF:
						BSP_LED_Off(led[led_arr_index]);
						break;
					default:
						break;
				}

				if (sendto(socket_fd, "done\n", sizeof("done\n"), 0,(struct sockaddr *)&client_addr,(socklen_t)addr_len) < 0)
				{
				 	TCP_SERVER_PRINTF("send() error\n");
				}

			}
			else
			{
				if (sendto(socket_fd, "Invalid command\n", sizeof("Invalid command\n"), 0,(struct sockaddr *)&client_addr,(socklen_t)addr_len) < 0)
				{
				 	TCP_SERVER_PRINTF("send() error\n");
				}
			}


		 }
		 else
		 {
			TCP_SERVER_PRINTF("recvfrom() error\n");
			close(socket_fd);
			return;

		 }


	}


}
