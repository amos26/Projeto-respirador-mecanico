/*
	Universidade Federal de Campina Grande
	Departamento de Engenharia Elétrica
	Disciplina: Laboratório de Arquitetura de Sistemas Digitais

	Aluno: Amós de Oliveira Batista
	Matrícula: 116 110 735
	
	SPRINT 9 - NOVAS FUNCIONALIDADES
 */ 

#include "respirador_mecanico.h"
#include <avr/interrupt.h>

// DADOS GERAIS
uint8_t valvula_automatica = 1; // Habilita controle automático da válvula de O2 ---------------------------------------
uint16_t tempo_ms = 0;
char sel = 's'; // Seleção de tela/parâmetros

// DADOS DE RESPIRAÇÃO
uint8_t freq_respiracao = FREQ_MIN, volume_respiracao = VOLUME_MAX;
uint16_t tempo_respiracao, valvula = 0;

// DADOS DE SINAIS VITAIS
uint16_t temperatura_corporal = 0, saturacao_O2 = 0, saturacao_antiga = 0, ultimo_batimento = 0, freq_batimentos = 0;
char pressao[7];

ISR(TIMER0_COMPA_vect) // Interrupção do timer TC0 (a cada 1ms)
{
	tempo_ms++;
	if (tempo_ms%tempo_respiracao == 0)
		respirar(volume_respiracao);
	if (tempo_ms%TEMPO_SENSORES_MS == 0)
		ADMUX ^= 0b00000001; // Alterna entre os sensores de temperatura corporal e saturação de O2
	if (tempo_ms%TEMPO_ATUALIZACAO_DISPLAY_MS == 0)
	{
		if (sel == 's')
			exibir_sinais_vitais(freq_batimentos, temperatura_corporal, saturacao_O2, pressao);
		else
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
	}
	if (tempo_ms%5000 == 0) // Checagem da saturacao de O2 a cada 1 minuto ------------------------------------------------------------
	{
		static uint8_t sat_caindo = 0;
		if (saturacao_O2 < saturacao_antiga)
		{
			sat_caindo++;
		}
		else
		{
			sat_caindo = 0;
			PORTB &= 0b11111110;
		}
		
		if (sat_caindo >= 3)
		{
			PORTB |= 0b00000001;
		}
		
		saturacao_antiga = saturacao_O2;
	}
}

ISR(PCINT2_vect) // Interrupção do pino PD4 (Sensor de batimentos cardíacos)
{
	static uint8_t subida = 1; 
	
	if (subida) // Controla contagem de batimentos (apenas bordas de subida)
	{
		freq_batimentos = 60000/(tempo_ms - ultimo_batimento);
		
		ultimo_batimento = tempo_ms;
		subida = 0;
	}
	else subida = 1;
}

ISR(PCINT0_vect) // Interrupção do pino PB6 (Botão de seleção de parâmetros (freq. respiração ou válvula de O2))
{
	static uint8_t descida = 1;
	
	if (descida) // Seleção de parâmetros de configuração
	{
		/*
			Seleção de tela/parâmetros (sel)
				's' = sinais vitais
				'r' = frequência de respiração
				'v' = válvula de O2
				'c' = controle automático de oxigênio injetado ---------------------------------------------------------------------
				'i' = ar injetado
		*/	
		switch(sel)
		{	
			case 's':
				sel = 'r';
				exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
			break;
			
			case 'r':
				sel = 'v';
				exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
			break;
			
			case 'v':
				sel = 'c';
				exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
			break;
			
			case 'c':
				sel = 'i';
				exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
			break;
			
			case 'i':
				sel = 's';
				exibir_sinais_vitais(freq_batimentos, temperatura_corporal, saturacao_O2, pressao);
			break;
			
			default:
				exibir_sinais_vitais(freq_batimentos, temperatura_corporal, saturacao_O2, pressao);
		}
		
		descida = 0;
	}
	else descida = 1;
}

// Interrupção externa INT0
ISR(INT0_vect) // Pino D2 aumenta a frequência de respiração ou abre 10% da válvula de O2
{
	if (sel == 'r')
	{
		if (freq_respiracao < FREQ_MAX)
		{
			freq_respiracao++;
			tempo_respiracao = CONSTANTE_RESPIRACAO/(volume_respiracao*freq_respiracao);
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'v')
	{
		valvula_automatica = 0; // Ao configurar a abertura da válvula manualmente, a opção automática é desabilitada -----------------------
		if (OCR1B < 4000)
		{
			OCR1B += 200;
			valvula += 10;
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'i')
	{
		if (volume_respiracao < VOLUME_MAX)
		{
			volume_respiracao++;
			tempo_respiracao = CONSTANTE_RESPIRACAO/(volume_respiracao*freq_respiracao);
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'c')
	{
		valvula_automatica ^= 1; // Controle de válvula (manual/automático) -------------------------------------------------
	}
}

// Interrupção externa INT1
ISR(INT1_vect) // Pino D3 diminui a frequência de respiração ou fecha 10% da válvula de O2
{
	if (sel == 'r')
	{
		if (freq_respiracao > FREQ_MIN)
		{
			freq_respiracao--;
			tempo_respiracao = CONSTANTE_RESPIRACAO/(volume_respiracao*freq_respiracao);
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'v')
	{
		valvula_automatica = 0; // Ao configurar a abertura da válvula manualmente, a opção automática é desabilitada -------------------------
		if (OCR1B > 2000)
		{
			OCR1B -= 200;
			valvula -= 10;
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'i')
	{
		if (volume_respiracao > VOLUME_MIN)
		{
			volume_respiracao--;
			tempo_respiracao = CONSTANTE_RESPIRACAO/(volume_respiracao*freq_respiracao);
			exibir_parametros(sel, freq_respiracao, valvula, valvula_automatica, volume_respiracao);
		}
	}
	else if (sel == 'c')
	{
		valvula_automatica ^= 1; // Controle de válvula (manual/automático) --------------------------------------------------------
	}
}

ISR(ADC_vect) // Interrupção do ADC
{
	uint16_t leitura_adc = ADC;
	
	if (ADMUX%2 == 0) // Leitura efetuada para o canal 0
	{
		temperatura_corporal = 0.0490196*leitura_adc + 10; // Equação da reta: Temperatura em função da tensão medida
	} 
	else // Leitura efetuada para o canal 1
	{
		saturacao_O2 = 0.122249*leitura_adc; // Equação da reta: Saturação de O2 em função da tensão medida
		
		if (valvula_automatica == 1) { // Se o controle automático estiver permitido --------------------------------------------
			uint16_t abertura = 0, abaixo = 0; // abaixo: nível de saturação abaixo do limite superior estabelecido
			
			if (saturacao_O2 > SATURACAO_LIM_INFERIOR && saturacao_O2 < SATURACAO_LIM_SUPERIOR)
			{
				abaixo = SATURACAO_LIM_SUPERIOR - saturacao_O2;
				abertura = (100*abaixo)/(SATURACAO_LIM_SUPERIOR - SATURACAO_LIM_INFERIOR); // Cálculo da abertura da válvula de O2, a partir da medida de saturação de O2 recebida
				abertura += 10 - abertura%10; // Ajuste de abertura, dada uma válvula que abre de 10 em 10 % ----------------------------------------------
			}
			else 
			{
				if (saturacao_O2 <= SATURACAO_LIM_INFERIOR) // Abaixo do limite máximo de saturação, a válvula abre 100% -----------------------------------
				{
					abertura = 100;
				}
				else if (saturacao_O2 >= SATURACAO_LIM_SUPERIOR) // Acima do limite aceitável de saturação, a válvula não precisa abrir --------------------
				{
					abertura = 0;
				}
			}
			
			OCR1B = 2000 + 20*abertura; // Controle do Servo Motor da válvula ----------------------------------------------------
			valvula = abertura; // Variável de exibição no display ---------------------------------------------------------------			
		}
	}
	
	if (temperatura_corporal < 35 || temperatura_corporal > 41 || saturacao_O2 < 60 || OCR1A == 2000)
	{
		PORTD |= 0b10000000; // Ativa o alarme
	}
	else
	{
		PORTD &= 0b01111111; // Desativa o alarme
	}
}

// Função para inicialização da USART
void USART_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char)(ubrr>>8); // Ajusta a taxa de transmissão
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0); // Habilita o transmissor e o receptor e interrupção ao receber
	UCSR0C = (3<<UCSZ00); // Modo Assíncrono / sem paridade / formato do frame: 8 bits de dados 1 bit de parada
}

// Interrupção da USART (ao receber dados de pressão)
ISR(USART_RX_vect)
{
	unsigned char recebido = UDR0;
	static char pressao_recebida[8];
	static int8_t posicao = -1, posicao_x = 0;
	
	if (posicao == -1)
	{
		if (recebido != ';')
			strcpy(pressao, "ERRO!");
		else
		{
			strcpy(pressao_recebida, "       ");
			posicao = 0;
		}
	}
	else if (recebido != ':')
	{
		if (recebido == ';')
		{
			strcpy(pressao, "ERRO!");
			strcpy(pressao_recebida, "       ");
			posicao = 0;
		}
		else
		{
			if (recebido == 'x') posicao_x = posicao;
			pressao_recebida[posicao] = recebido;
			posicao++;
		}
	}
	else
	{
		if (posicao < 3 || posicao > 7 || posicao_x == 0 || posicao_x > 3 || posicao-posicao_x < 2 || posicao-posicao_x > 4)
			strcpy(pressao, "ERRO!");
		else if (pressao_recebida[0] == '-' || pressao_recebida[posicao_x+1] == '-')
			strcpy(pressao, "ERRO!");
		else
		{
			strcpy(pressao, "       ");
			strcpy(pressao, pressao_recebida);
		}
		
		strcpy(pressao_recebida, "       ");
		posicao = -1;
	}
}

int main(void) 
{
	// Configuração de entradas
	DDRB &= 0b10111111; // Habilita o pino B6 como entrada (B6 Seleção de parâmetros)
	PORTB |= 0b01000000; // Habilita resistor de pull-up do pino B6
	DDRD &= 0b11100000; // Habilita os pinos D0 a D4 como entradas (D0,D1 UART / D2,D3 +-resp / D4 fcardiaca)
	PORTD |= 0b00001100; // Habilita os resistores de pull-up dos pinos D2 e D3
	DDRC &= 0b11111100; // Habilita os pinos C0 e C1 como entradas (C0 Temperatura corporal / C1 Saturação O2)
	PORTC &= 0b11111100; // Desabilita os resistores de pull-up dos pinos C0 e C1
	
	// Configuração de saídas
	DDRB |= 0b00000111; // Habilita os pinos B0 (aviso de saturação caindo), B1 e B2 como saídas (PWMs que controlam respiração e válvula de O2)
	DDRC |= 0b11111100; // Habilita os pinos C2 a C6 como saídas (LCD)
	DDRD |= 0b10000000; // Habilita a porta D7 como saída (Buzzer)
		
	// Configuração do Timer 0 (contagem em ms)
	TCCR0A = 0b00000010; // Habilita modo de operação CTC do TC0
	TCCR0B = 0b00000011; // Prescaler P = 64
	OCR0A = 249; // Ajusta o comparador para TC0 contar até C = 249
	TIMSK0 = 0b00000010; // Habilita interrupção na igualdade da comparação com OCR0A (t = (P*(C+1))/f = 1ms)
	
	// Configuração de PWMs com Timer 1
	// Modo PWM rápido via ICR1, F_PWM = 50Hz, prescaler = 8, TOP = (F_CPU/(prescaler*F_PWM))-1 = 39999
	ICR1 = 39999; // F_PWM = 50Hz => T_PWM = 20ms
	TCCR1A = 0b10100010; // PWM rápido via ICR1, não-invertido, saídas OC1A (PB1) OC1B (PB2)
	TCCR1B = 0b00011010; // prescaler = 8
	OCR1A = 2000; // canal A (PB1) inicia na posição 0 / Controla respiração (servo BVM)
	OCR1B = 2000; // canal B (PB2) inicia na posição 0 / Controla válvula de O2
	
	// Configuração de Interrupções Externas
	EICRA = 0b00001010; // Ativa interrupções INT0 e INT1 na borda de descida
	EIMSK = 0b00000011; // Habilita as interrupções externas INT0 e INT1
	PCICR = 0b00000101; // Habilita interrupções nas portas B e D
	PCMSK2 = 0b00010000; // Habilita inrrupções no pino D4 (Sensor de batimentos cardíacos)
	PCMSK0 = 0b01000000; // Habilita inrrupções no pino B6 (Botão de seleção de parâmetros)
	
	// Configuração do ADC
	ADMUX = 0b01000000; // tensão de referência AVCC (pino A+), canal 0 (temp. corporal)
	ADCSRA = 0b11101111; // Habilita o AD, habilita interrupção, modo de conversão contínua, prescaler = 128
	ADCSRB = 0x00; // Modo de conversão contínua
	DIDR0 = 0b00111100; // Habilita PC0 e PC1 como entradas do ADC0
	
	USART_Init(MYUBRR); // Inicialização da USART

	sei(); // Habilita interrupções globais
	
	tempo_respiracao = CONSTANTE_RESPIRACAO/(volume_respiracao*freq_respiracao);
		
	nokia_lcd_init(); // Inicialização do display Nokia 5110 PCD8544
	exibir_sinais_vitais(freq_batimentos, temperatura_corporal, saturacao_O2, pressao);
	
    while (1) 
    {
	}
}