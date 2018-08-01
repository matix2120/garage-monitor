#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "packet.h"

enum {
	ALARM = 0,
	ENV_UPDATE,
};

static bool is_crc_ok(char *packet, uint8_t length)
{
	uint16_t crc = 0;

	for (int i = 0; i < length-2; i++)
		crc += packet[i];

	crc = crc%10;

	if (packet[length - 1] - '0'  == crc)
	{
		printf("debug: crc %d, packet crc %d\n", crc, packet[length - 1] - '0');
		return true;
	}
	else
	{
		printf("debug: crc %d, len: %d\n", crc, length);
	}

	return false;
}

static int http_send(char *ip, char *request)
{
    int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};


	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Error: %d - %s\n", errno, strerror(errno));
		return -1;
	}
	memset(&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);

	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
	{
		printf("Error: %d - %s\n", errno, strerror(errno));
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Error: %d - %s\n", errno, strerror(errno));
		return -1;
	}

	send(sock, request, strlen(request), 0);
	read(sock, buffer, 1024);
	close(sock);
	/*if (strstr(buffer, "Congratulations!") == NULL)
	{
		printf("%s\n", buffer);
		return -1;
	}
	*/
	return 0;
}


static int ifttt_notification()
{
	printf("IFTTT notification\n");
	return http_send(IFTTT_IP, IFTTT_REQUEST);
}

static int domoticz_update(double temperature, double voltage, long int snr)
{
	char buffer[128];

	sprintf(buffer, "GET /json.htm?type=command&param=udevice&idx=12&nvalue=0&svalue=%f HTTP/1.1\r\nhost: %s\r\n\r\n", temperature, DOMOTICZ_IP);
	if (http_send(DOMOTICZ_IP, buffer))
		return -1;

	sprintf(buffer, "GET /json.htm?type=command&param=udevice&idx=13&nvalue=0&svalue=%f HTTP/1.1\r\nhost: %s\r\n\r\n", voltage, DOMOTICZ_IP);
	if(http_send(DOMOTICZ_IP, buffer))
		return -1;

	sprintf(buffer, "GET /json.htm?type=command&param=udevice&idx=14&nvalue=0&svalue=%li HTTP/1.1\r\nhost: %s\r\n\r\n", snr, DOMOTICZ_IP);
	if(http_send(DOMOTICZ_IP, buffer))
		return -1;
	return 0;
}

int decode_packet(char *packet, long int snr, uint8_t length)
{
	uint8_t packet_type;
	char *p;
	double temperature, voltage;

	if(!is_crc_ok(packet, length))
	{
		printf("bad crc\n");
		return -1;
	}

	p = strtok(packet, ",");
	packet_type = atoi(p);
	if (packet_type == ALARM)
	{
		ifttt_notification();
	}
	else if (packet_type == ENV_UPDATE)
	{
		p = strtok(NULL, ",");
		if (p != NULL)
			temperature = atof(p);
		else {
			printf("Error decoding temperature\n");
			return -1;
		}

		p = strtok(NULL, ",");
		if (p != NULL)
			voltage = atof(p);
		else {
			printf("Error decoding voltage\n");
			return -1;
		}

		domoticz_update(temperature, voltage, snr);
	}
	else
	{
		printf("Unsupported packet type (%d)\n", packet_type);
		return -1;
	}
	return 0;
}

/*int main ()
{
	double temperature, voltage;
    char tab[] = "1,27.1,14.52,5";
	char tab2[] = "0,8";
	if (!decode_packet(tab, &temperature, &voltage))
		printf("temp: %.1f, volt: %.2f\n", temperature, voltage);
	if (!decode_packet(tab2, &temperature, &voltage))
		printf("temp: %.1f, volt: %.2f\n", temperature, voltage);
    return 0;
}*/
