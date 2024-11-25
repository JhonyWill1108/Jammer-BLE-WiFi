// ESP32 S3 WROOM 1
// 2x Modulo NRF24L01 Com Antena SMA

/* 
Pinout (ajustar de acordo com os pinos do ESP)

Consultar linhas 136 e 156

	HSPI
		SCK = 14
		MISO = 12
		MOSI = 13
		CS = 15
		CE = 16
	VSPI
		SCK = 18
		MISO = 19
		MOSI = 23
		CS = 21
		CE = 22
*/

// BIBLIOTECAS
#include "RF24.h"
#include <SPI.h>
#include <ezButton.h>	//Para adicionar um botão de power
#include "esp_bt.h"
#include "esp_wifi.h"
#include <FastLED.h>	//Para o LED

// Configurações do LED RGB
#define LED_PIN 48			// Pino onde o LED está conectado (verifique o pino correto no manual)
#define NUM_LEDS 1			// Número de LEDs

CRGB leds[NUM_LEDS];

// Declaração de protótipos
void initSP();
void initHP();

SPIClass *sp = nullptr;
SPIClass *hp = nullptr;

RF24 radio(16, 15, 16000000);   			// HSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000
RF24 radio1(22, 21, 16000000);  			// VSPI CAN SET SPI SPEED TO 16000000 BY DEFAULT ITS 10000000

unsigned int flag = 0;					// HSPI Variável de sinalização para monitorar a direção
unsigned int flagv = 0;					// VSPI Variável de sinalização para monitorar a direção
int ch = 45;						// Variável para armazenar valor de ch
int ch1 = 45;						// Variável para armazenar valor de ch

ezButton toggleSwitch(33);

void two() {

	if (flagv == 0) {  
    		ch1 += 4;				// Se a bandeira for 0, incremente ch em 4 e ch1 em 1
  	} else {  				
    		ch1 -= 4;				// Se a bandeira não for 0, diminua ch em 4 e ch1 em 1
  	}

	if (flag == 0) {  
		ch += 2;				// Se a bandeira for 0, incremente ch em 4 e ch1 em 1
	} else {  
		ch -= 2;				// Se a bandeira não for 0, diminua ch em 4 e ch1 em 1
	}

  	// Verifique se ch1 é maior que 79 e o sinalizador é 0
	if ((ch1 > 79) && (flagv == 0)) {
		flagv = 1;				// Set flag to 1 to change direction
	} else if ((ch1 < 2) && (flagv == 1)) {  	// Check if ch1 is less than 2 and flag is 1
		flagv = 0;				// Set flag to 0 to change direction
	}

  	// Verifique se ch é maior que 79 e o sinalizador é 0
	if ((ch > 79) && (flag == 0)) {
		flag = 1;                            	// Defina o sinalizador como 1 para mudar de direção
	} else if ((ch < 2) && (flag == 1)) {		// Verifique se ch é menor que 2 e o sinalizador é 1
		flag = 0;                            	// Defina o sinalizador como 0 para mudar de direção
	}

  	radio.setChannel(ch);
  	radio1.setChannel(ch1);
  	
	/* 
	Serial.print("SP:");
  	Serial.println(ch1);
  	Serial.print("\tHP:");
  	Serial.println(ch);
	*/
}

void one() {

	//CANAL ALEATORIO
	radio1.setChannel(random(80));
	radio.setChannel(random(80));
	delayMicroseconds(random(60));			// REMOVE IF SLOW

	// VOCÊ PODE FAZER VARREDURA DE CANAL
	/*
	for (int i = 0; i < 79; i++) {
	radio.setChannel(i);
	*/
}

void setup() {

	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS); // Inicializa o LED

	Serial.begin(115200);
	esp_bt_controller_deinit();
	esp_wifi_stop();
	esp_wifi_deinit();
	esp_wifi_disconnect();
	toggleSwitch.setDebounceTime(50);

	initHP();
	initSP();
}

void initSP() {

	sp = new SPIClass(1);		// 1 corresponde ao VSPI
	sp->begin(12, 13, 11, 10);		// SCK, MISO, MOSI, SS - Os números correspondem aos pinos do ESP

  	if (radio1.begin(sp)) 
	{
    		Serial.println("SP iniciado!!!");
    		radio1.setAutoAck(false);
    		radio1.stopListening();
    		radio1.setRetries(0, 0);
		radio1.setPALevel(RF24_PA_MAX, true);
    		radio1.setDataRate(RF24_2MBPS);
    		radio1.setCRCLength(RF24_CRC_DISABLED);
    		radio1.printPrettyDetails();
    		radio1.startConstCarrier(RF24_PA_MAX, ch1);
  	} else {
    		Serial.println("O SP não pôde iniciar!!!");
  	}
}

void initHP() {

    hp = new SPIClass(2);		// 2 corresponde ao HSPI
    hp->begin(22, 23, 21, 20);		// SCK, MISO, MOSI, SS - Os números correspondem aos pinos do ESP

  	if (radio.begin(hp)) {
    		Serial.println("HP iniciado!!!");
    		radio.setAutoAck(false);
    		radio.stopListening();
    		radio.setRetries(0, 0);
    		radio.setPALevel(RF24_PA_MAX, true);
    		radio.setDataRate(RF24_2MBPS);
    		radio.setCRCLength(RF24_CRC_DISABLED);
    		radio.printPrettyDetails();
    		radio.startConstCarrier(RF24_PA_MAX, ch);
  	} else {
    		Serial.println("O HP não pôde iniciar!!!");
  	}
}

void loop() {

// INICIO LED
	// Inicializa o LED
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);

	// Inicio teste efeito CHROMA
	
	static uint8_t hue = 0;		// Variável de matiz (ciclo de cores)
	
	// Atualiza a cor do LED baseado no matiz
	leds[0] = CHSV(hue, 255, 255);  // Cor dinâmica (HSV) 
		/*
		CHSV(hue, 255, 255):
		
		* hue: Controla a matiz (ciclo de núcleos).
		* 255(saturação) : Define a intensidade da cor (0 seria tons de cinza). (0 a 255)
		* 255(brilho) : Define a intensidade do brilho. (0 a 255)
		*/

	FastLED.show();
	
	hue++;				// Incrementa o matiz para o próximo ciclo
	
	delay(10);			// Tempo de transição entre cores
	
	// Fim teste efeito CHROMA

/*
	// Acende o LED em vermelho
	leds[0] = CRGB::Red;
  	FastLED.show();
  	delay(1000);

  	// Acende o LED em verde
  	leds[0] = CRGB::Green;
  	FastLED.show();
  	delay(1000);

  	// Acende o LED em azul
  	leds[0] = CRGB::Blue;
  	FastLED.show();
  	delay(1000);

  	// Acende o LED em branco (todas as cores)
  	leds[0] = CRGB::White;
	FastLED.show();
	delay(1000);

	// Faz o LED apagar
	leds[0] = CRGB::Black;
	FastLED.show();
	delay(1000);
/*

// FIM LED

	toggleSwitch.loop();		// DEVE chamar a função loop() primeiro

	/* 
	if (toggleSwitch.isPressed())
		Serial.println("one");
	if (toggleSwitch.isReleased())
		Serial.println("two");
	*/

	int state = toggleSwitch.getState();

	if (state == HIGH)
		two();
	else {
		one();
  	}
}
