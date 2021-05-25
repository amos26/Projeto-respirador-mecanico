#define FREQ_MIN 5
#define FREQ_MAX 30
#define VOLUME_MIN 1
#define VOLUME_MAX 8
#define SATURACAO_LIM_SUPERIOR 95 // Valor de saturação de O2 a partir do qual é necessária injeção de Oxigênio ----------------------------
#define SATURACAO_LIM_INFERIOR 70 // Valor de saturação de O2 a partir do qual a válvula de O2 é aberta 100%--------------------------------
#define CONSTANTE_RESPIRACAO 30000 // (60*1000)/2
#define TEMPO_SENSORES_MS 150
#define TEMPO_ATUALIZACAO_DISPLAY_MS 200
#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include "nokia5110.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

void respirar(uint8_t volume_respiracao); // Acende ou apaga LEDs, de acordo com o estado da respiração
void exibir_parametros(char sel, uint16_t frequencia, uint16_t valvula, uint8_t valvula_automatica, uint8_t volume_respiracao); // Exibe e configura os parâmetros de respiração
void exibir_sinais_vitais // Exibe as frequências de respiração e batimentos cardíacos, temperatura e saturação no LCD
(
	uint16_t batimentos,
	uint16_t temperatura_corporal,
	uint16_t saturacao_O2,
	char pressao[7]
);