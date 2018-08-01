#ifndef __CRC_H__
#define __CRC_H__


#include <stdlib.h>


#define DOMOTICZ_IP "192.168.1.20"
#define IFTTT_IP "34.227.75.13"
#define IFTTT_REQUEST "GET /trigger/garage_opened/with/key/Hf2dw3Uc-779JIsRAZSyw HTTP/1.1\r\nhost: maker.ifttt.com\r\n\r\n"

int decode_packet(char *packet, long int snr, uint8_t length);
#endif /* __CRC_H__ */
