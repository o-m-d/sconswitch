/*************************************************************************
 * Title:           Serial Controlled Switch
 * Author:          Olexandr Davydenko
 * Description:     32 output channels, controlled by commands via serial port.
*************************************************************************/

#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "main.h"

#include "uart.h"

// MCUCPP
#include "iopins.h"
#include "pinlist.h"

using namespace Mcucpp::IO;

typedef PinList <Pb7, Pb6, Pb5, Pb4, Pb3, Pb2, Pb1, Pb0, Pa0, Pa1, Pa2, Pa3, Pa4, Pa5, Pa6, Pa7, Pe0, Pe1, Pe2, Pc7, Pc6, Pc5, Pc4, Pc3, Pc2, Pc1, Pc0, Pd7, Pd6, Pd5, Pd4, Pd3> mainport;

const int8_t mainport_width = mainport::PinListProperties::Length - 1;
uint32_t mainport_data;

void cleanbuf(char *arr_start, char *arr_end);
void actionchan(uint8_t chan, char action);
void togglechan(uint8_t chan, uint16_t delay);
void parse_cmndline(char *command_part[], char *buffer);
void showportstatus();

int main(void)
{
    uint16_t read_char; //because UART_* errors are 16 bit
    uint16_t delay;
    char buffer[UART_CMND_SIZE], *command_part[MAX_CMDARG_COUNT], command;
    uint8_t charcount = 0, chan;

    mainport::SetConfiguration(mainport::Out);

    // check if this fist work with empty EEPROM
    if (eeprom_read_byte(EE_FLAG_NEW) == 0xff)
    {
        eeprom_write_byte(EE_FLAG_NEW, 0);
        eeprom_write_dword(EE_BASE_PORT, 0);
    }
    mainport_data = eeprom_read_dword(EE_BASE_PORT);
    mainport::Write(mainport_data);

    wdt_reset();
    wdt_enable(WDTO_2S);

    uart_init(UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU));
    sei();
    cleanbuf(buffer, buffer+UART_CMND_SIZE);
    U_HELP;
    U_PROMPT;

    for (;;)
    {
        wdt_reset();
        // get char and verify UART status
        read_char = uart_getc();
        if (read_char == UART_NO_DATA)
        {
        }
        else
        {
            if (read_char == UART_FRAME_ERROR)
            {
                U_F;
            }
            if (read_char == UART_OVERRUN_ERROR)
            {
                U_OR;
            }
            if (read_char == UART_BUFFER_OVERFLOW)
            {
                U_OF;
            }

            // Enter pressed
            if ((read_char == 0x0a) || (read_char == 0x0d))
            {
                // parse commandline
                parse_cmndline(command_part, buffer);

                // first argument
                if (command_part[0])
                {
                    command = tolower(*command_part[0]);
                    switch (command)
                    {
                        case 'h':
                        case '?':
                            U_HELP;
                            break;

                        case 's':
                            showportstatus();
                            break;

                        case 'a':
                        case 'p':
                            chan = atoi(command_part[1]);
                            if ((LOWCHAN > chan) || (chan > mainport_width) || (!isdigit(*command_part[1]) && (tolower(*command_part[1]) != 'a')))
                            {
                                U_ERR;
                                goto ENDPARSE;
                            }
                            if (tolower(*command_part[1]) == 'a')
                            {
                                if (command == 'p')
                                {
                                    mainport_data = 0;
                                }
                                if (command == 'a')
                                {
                                    mainport_data = 0xffffffffUL;
                                }
                                mainport::Write(mainport_data);
                                goto end_of_switch;
                            }
                            actionchan(chan, command);

                            end_of_switch:
                            eeprom_write_dword(EE_BASE_PORT, mainport_data);
                            break;

                        case 't':
                            // find second argument - channel num
                            chan = atoi(command_part[1]);
                            if ((LOWCHAN > chan) || (chan > mainport_width) || !isdigit(*command_part[1]))
                            {
                                U_ERR;
                                goto ENDPARSE;
                            }
                            // find third argument - delay
                            delay = atoi(command_part[2]);
                            if ((LOWDLY > delay) || (delay > HIGHDLY))
                            {
                                U_ERR;
                                goto ENDPARSE;
                            }
                            togglechan(chan, delay);
                            break;

                        case 'n':
                            // find second argument - channel num
                            chan = atoi(command_part[1]);
                            if ((LOWCHAN > chan) || (chan > mainport_width) || !isdigit(*command_part[1]))
                            {
                                U_ERR;
                                goto ENDPARSE;
                            }
                            // find third argument - channel name
                            if ((strlen(command_part[2]) > CHAN_NAME_SIZE))
                            {
                                U_ERR;
                                goto ENDPARSE;
                            }
                            else if (!command_part[2])
                            {
                                // erase channel name
                                int8_t name_char = CHAN_NAME_SIZE;
                                while (name_char > 0)
                                {
                                    name_char--;
                                    eeprom_write_byte((uint8_t*)(EE_BASE_NAMES + (chan * CHAN_NAME_SIZE) + name_char), 0xff);
                                }

                                goto ENDPARSE;
                            }
                            else
                            {
                                // save channel name
                                eeprom_write_block((const void*)command_part[2],  (void*)(EE_BASE_NAMES + (chan * CHAN_NAME_SIZE)), CHAN_NAME_SIZE);
                                goto ENDPARSE;
                            }
                            break;

                        default:
                            U_ERR;
                            U_HELP;
                            goto ENDPARSE;
                    }
                }

ENDPARSE:
                charcount = 0;
                cleanbuf(buffer, buffer+UART_CMND_SIZE);
                U_PROMPT;
            }
            else
            {
                // Accumulate chars of command line
                if (charcount < UART_CMND_SIZE - 1)
                {
                    uart_putc((unsigned char)read_char);
                    buffer[charcount] = read_char;
                    charcount++;
                }
                // Detect command line oversize
                else
                {
                    charcount = 0;
                    cleanbuf(buffer, buffer+UART_CMND_SIZE);
                    U_CMNDLONG;
                    U_PROMPT;
                }
            }
        }
    }
}

void cleanbuf(char *arr_start, char *arr_end)
{
    while (arr_start < arr_end)
    {
        *(arr_start++) = 0;
    }
}

void actionchan(uint8_t chan, char action)
{
    uint32_t ui32Chanstatus;
    ui32Chanstatus = mainport_data;

    if (action == 'p')
    {
        mainport_data = (ui32Chanstatus &= ~(1UL << chan));
        mainport::Write(mainport_data);
    }
    else if (action == 'a')
    {
        mainport_data = (ui32Chanstatus |= (1UL << chan));
        mainport::Write(mainport_data);
    }
    else
    {
        U_ERR;
    }
}

void showportstatus()
{
    int8_t i;
    char cPortstatus[UART_CMND_SIZE], cChanName[CHAN_NAME_SIZE + 1];

    for (i = mainport_width; i >= 0; i--)
    {
        uart_puts_P("\r\nOutput ");
        uart_puts(itoa(i, cPortstatus, 10));
        uart_puts_P("\t");

        eeprom_read_block((void *) cChanName, (const void *) (EE_BASE_NAMES + (i * CHAN_NAME_SIZE)), CHAN_NAME_SIZE);
        if (cChanName[0] != 0xff)
        {
            cChanName[CHAN_NAME_SIZE] = 0;
            uart_puts(cChanName);
            if (strlen(cChanName) < CHAN_NAME_SIZE)
            {
                uart_puts_P("\t");
            }
        }
        else
        {
            uart_puts_P("\t");
        }

        if (mainport_data & (1UL << i))
        {
            uart_puts_P("\tActive");
        }
        else
        {
            uart_puts_P("\tPassive");
        }
    }
}

void togglechan(uint8_t chan, uint16_t delay)
{
    uint32_t ui32tmp = mainport_data;
    uint16_t read_char;
    char cPortstatus[UART_CMND_SIZE];

    if (mainport_data & (1UL << chan))
    {
        mainport::Write(ui32tmp &= ~(1UL << chan));
    }
    else
    {
        mainport::Write(ui32tmp |= (1UL << chan));
    }

    uart_puts_P("\r\nStart toggle. Press s for stop. Remain ");

    while (delay > 0)
    {
        uart_puts(itoa(delay, cPortstatus, 10));
        uart_puts_P("\e[K"); //erase to end of line
        wdt_reset();
        _delay_ms(250);
        wdt_reset();
        _delay_ms(250);
        wdt_reset();
        _delay_ms(250);
        wdt_reset();
        _delay_ms(250);
        delay--;

        // return cursor to position for display numbers
        if (delay >= 99)
        {
            uart_puts_P("\b\b\b");
        }
        else if (delay >= 9)
        {
            uart_puts_P("\b\b");
        }
        else
        {
            uart_puts_P("\b");
        }

        read_char = uart_getc();
        if (read_char == UART_NO_DATA)
        {
        }
        else
        {
            if ((read_char == 0x53) || (read_char == 0x73))
            {
                break;
            }
        }
    }
    if (delay == 0)
    {
        uart_puts_P("\r\nDone");
    }
    else
    {
        uart_puts_P("\r\nStopped");
    }
    mainport::Write(mainport_data);
}

void parse_cmndline(char *command_part[], char *buffer)
{
    int8_t command_part_count = 0;
    command_part[command_part_count] = strtok(buffer, " ");
    while (command_part_count < MAX_CMDARG_COUNT)
    {
        command_part_count++;
        command_part[command_part_count] = strtok(NULL, " ");
    }
}
