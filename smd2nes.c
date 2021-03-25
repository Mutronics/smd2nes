#include "defines.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include "bits.h"
#include "gamepad.h"

volatile uint8_t button_data;
volatile uint8_t shift;
volatile uint8_t turbo;

// Clock
ISR(INT0_vect)
{
	shift <<= 1;
	if (button_data & shift)
		PORTD = 1;
	else
		PORTD = 0;
}

// Strobe
ISR(INT1_vect)
{
	shift = 1;
	if (button_data & shift)
		PORTD = 1;
	else
		PORTD = 0;
	turbo++;
	turbo %= 4;
}

int main (void)
{
	set_bit(DDRD, 0); // Данные - на выход
	unset_bit2(DDRD, 2, 3); // clock, strobe - на ввод
	DDRB = 0; // Кнопки на ввод
	PORTB = 0xFF; // Подтяжка кнопок
	set_bit2(MCUCR, ISC11, ISC10); // Прерывание при растущем strobe
	set_bit2(MCUCR, ISC01, ISC00); // Прерывание при растущем  clock
	set_bit(GICR, INT0); set_bit(GICR, INT1); // Активируем их
	init_smd_gamepad();
	sei(); // Глобальная активация прерываний
	
	// Right, Left, Down, Up, Start, Select, B, A
	while(1)
	{
		uint8_t nes_temp_data = 0xff;
		int b, c;
		for (c = 0; c < 4; c++)
		{
			uint16_t smd_gamepad_data = get_smd_gamepad();
			if ((smd_gamepad_data & 0b00001111) || (c < 2)) // 3-button mode
			{
				for (b = 0; b <= 13; b++)
				{
					if (!((smd_gamepad_data>>b)&1))
					{
						switch (b)
						{
							case 0: // Up
								unset_bit(nes_temp_data, 4);
								break;
							case 1: // Down
								unset_bit(nes_temp_data, 5);
								break;
							case 4: // A(SMD)/A+B(NES)
								unset_bit(nes_temp_data, 0);
								unset_bit(nes_temp_data, 1);
								break;
							case 5: // Start
								unset_bit(nes_temp_data, 3);
								break;
							case 10: // Left
								unset_bit(nes_temp_data, 6);
								break;
							case 11: // Right
								unset_bit(nes_temp_data, 7);
								break;
							case 12: // B(SMD)/B(NES)
								unset_bit(nes_temp_data, 1);
								break;
							case 13: // C(SMD)/A(NES)
								unset_bit(nes_temp_data, 0);
								break;
						}
					}
				}
			} else { // 6-button mode
				for (b = 4; b <= 11; b++)
				{
					if (!((smd_gamepad_data>>b)&1))
					{
						switch (b)
						{
							case 4: // A(SMD)/Select(NES)
								unset_bit(nes_temp_data, 0);
								unset_bit(nes_temp_data, 1);
								break;
							case 5: // Start
								unset_bit(nes_temp_data, 3);
								break;
							case 8: // Z(SMD)/Turbo-A(NES)
								if (turbo >= 2)
									unset_bit(nes_temp_data, 0);
								break;
							case 9: // Y(SMD)/Turbo-B(NES)
								if (turbo >= 2)
									unset_bit(nes_temp_data, 1);
								break;
							case 10: // X(SMD)/Select(NES)
								if (turbo >= 2) {
									unset_bit(nes_temp_data, 0);
									unset_bit(nes_temp_data, 1);
								}
								break;
							case 11: // Mode(SMD)/Select(NES)
								unset_bit(nes_temp_data, 2);
								break;
						}
					}
				}
			}
		}
		button_data = nes_temp_data;
		_delay_ms(2);
	}
}