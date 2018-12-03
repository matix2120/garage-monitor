#include <SPI.h>
#include <LoRa.h>
#include <OneWire.h>
#include <DallasTemperature.h>

uint8_t datasend[72];
double voltage, temperature;
char voltage_str[12]={"\0"}, temperature_str[12]={"\0"};
OneWire oneWire(A0);
DallasTemperature sensors(&oneWire);

void setup() {
  
  Serial.begin(115200);
  while (!Serial);

  Serial.println("SQ8NXC LORA Transmitter");

  while (!LoRa.begin(8681E5)) {
    Serial.println("Starting LoRa failed, retrying...");
  }

  sensors.begin();
}

uint8_t calculate_crc(uint8_t *buffer)
{
  uint16_t crc = 0;
  int i = 0;
  while (buffer[i] != 0) {
    crc += buffer[i++];
  }
  return crc % 10;
}


void loop() {
  static uint8_t counter = 60;
  uint16_t raw_voltage = 0;

  counter++;
  if (counter >= 60)
  {
    Serial.print('\n');
    counter = 0;
    for (int i = 0; i < 10; i++) {
      raw_voltage += analogRead(2);
    }
    voltage = raw_voltage * 1.48 / 1000.0;
  
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);
    
    dtostrf(voltage, 3, 2, voltage_str);
    dtostrf(temperature, 3, 1, temperature_str);
    sprintf(datasend, "1,%s,%s", temperature_str, voltage_str);
    sprintf(datasend, "%s,%d", datasend, calculate_crc(datasend));
    
    Serial.println((char *)datasend);
  
    LoRa.beginPacket();
    LoRa.print((char*)datasend);
    LoRa.endPacket();
  }
  delay(1000);
  Serial.print('.');
}

