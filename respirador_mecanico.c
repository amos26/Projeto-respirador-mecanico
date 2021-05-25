#include "respirador_mecanico.h"

void respirar(uint8_t volume_respiracao)
{
	static uint8_t inspirando = 1, posicao = 0;
	
	if (inspirando)
	{
		OCR1A += 250; // Incrementa a posição do servo em 2000/8
		if (posicao < volume_respiracao-1) posicao++; // Posição do servo varia de 0 a 7
		else inspirando = 0;
	}
	else
	{
		OCR1A -= 250; // Decrementa a posição do servo em 2000/8
		if (posicao > 0) posicao--; // Posição do servo varia de 0 a 7
		else inspirando = 1;
	}
	
	if (OCR1A == 2000)
	{
		PORTD |= 0b10000000; // Ativa o alarme
	}
	else
	{
		PORTD &= 0b01111111; // Desativa o alarme
	}
}

void exibir_sinais_vitais(uint16_t batimentos, uint16_t temperatura_corporal, uint16_t saturacao_O2, char pressao[7])
{
	char s_batimentos[3], s_temperatura_corporal[4], s_saturacao_O2[4];
	
	nokia_lcd_clear();
	
	// Exibição de Sinais Vitais
	nokia_lcd_set_cursor(0,0);
	nokia_lcd_write_string("SINAIS VITAIS",1);
	
	// Exibição da frequência de batimentos cardíacos (bpm)
	itoa(batimentos, s_batimentos, 10);
	nokia_lcd_set_cursor(0,10);
	nokia_lcd_write_string(s_batimentos,1);
	nokia_lcd_set_cursor(40,10);
	nokia_lcd_write_string("bpm",1);
	
	// Exibição da temperatura corporal (°C)
	itoa(temperatura_corporal, s_temperatura_corporal, 10);
	nokia_lcd_set_cursor(0,20);
	nokia_lcd_write_string(s_temperatura_corporal,1);
	nokia_lcd_set_cursor(40,20);
	nokia_lcd_write_string("°C",1);
	
	// Exibição da saturação de O2 (%SpO2)
	itoa(saturacao_O2, s_saturacao_O2, 10);
	nokia_lcd_set_cursor(0,30);
	nokia_lcd_write_string(s_saturacao_O2,1);
	nokia_lcd_set_cursor(40,30);
	nokia_lcd_write_string("%SpO2",1);
	
	// Exibição da pressão arterial (mmHg)
	nokia_lcd_set_cursor(0,40);
	nokia_lcd_write_string(pressao,1);
	nokia_lcd_set_cursor(45,40);
	nokia_lcd_write_string("mmHg",1);
	
	nokia_lcd_render();
}

void exibir_parametros(char sel, uint16_t frequencia, uint16_t valvula, uint8_t valvula_automatica, uint8_t volume_respiracao)
{
	char s_frequencia[3], s_valvula[4], s_valvula_automatica[3], s_volume_respiracao[2];
	
	nokia_lcd_clear();
	
	// Exibição de Parâmetros
	nokia_lcd_set_cursor(0,0);
	nokia_lcd_write_string("PARAMETROS",1);
	
	// Exibição da frequência de respiração (rsp/min)
	itoa(frequencia, s_frequencia, 10);
	nokia_lcd_set_cursor(0,10);
	nokia_lcd_write_string(s_frequencia,1);
	nokia_lcd_set_cursor(40,10);
	nokia_lcd_write_string("rsp/min",1);
	
	// Exibição da Abertura da válvula de O2 (%O2)
	itoa(valvula, s_valvula, 10);
	nokia_lcd_set_cursor(0,20);
	nokia_lcd_write_string(s_valvula,1);
	nokia_lcd_set_cursor(40,20);
	nokia_lcd_write_string("%O2",1);
	
	// Exibição do Controle da válvula de O2
	if (valvula_automatica == 1)
		strcpy(s_valvula_automatica, "Auto.");
	else
		strcpy(s_valvula_automatica, "Man.");
	nokia_lcd_set_cursor(0,30);
	nokia_lcd_write_string(s_valvula_automatica,1);
	nokia_lcd_set_cursor(40,30);
	nokia_lcd_write_string("Valvula",1);

	// Exibição do Volume de ar injetado (vol)
	itoa(volume_respiracao, s_volume_respiracao, 10);
	nokia_lcd_set_cursor(0,40);
	nokia_lcd_write_string(s_volume_respiracao,1);
	nokia_lcd_set_cursor(40,40);
	nokia_lcd_write_string("vol",1);

	if (sel == 'r')
	{
		nokia_lcd_set_cursor(30,10);
	}
	else if (sel == 'v')
	{
		nokia_lcd_set_cursor(30,20);
	}
	else if (sel == 'c')
	{
		nokia_lcd_set_cursor(30,30);
	}
	else if (sel == 'i')
	{
		nokia_lcd_set_cursor(30,40);
	}
	
	nokia_lcd_write_string("*",1);
	nokia_lcd_render();
}