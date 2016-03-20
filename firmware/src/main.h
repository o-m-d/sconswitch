/*************************************************************************
 * Title:           Serial Controlled Switch
 * Author:          Olexandr Davydenko
 * Description:     32 output channels, controlled by commands via serial port.
*************************************************************************/

# ifndef _MAIN_H
# define _MAIN_H

#define LOWCHAN                     0

#define LOWDLY                      1
#define HIGHDLY                     255

#define EE_BASE_PORT                (uint32_t *)(0x10)
#define EE_FLAG_NEW                 (uint8_t *)(0x08)
#define EE_BASE_NAMES               0x20
#define CHAN_NAME_SIZE              8
#define MAX_CMDARG_COUNT            5    //don't forget proper init array elements

#define UART_BAUD_RATE              9600
#define UART_CMND_SIZE              32

//UART messages
#define U_HELP uart_puts_P("\r\n\r\n\r\n\t\t-= SConSwitch =-\r\nAvailable commands:\r\n\ta N|a\t- Active output number N (or all)\r\n\tp N|a\t- Passive output number N (or all)\r\n\tt N D\t- Toggle output N for D (1-255) seconds, press s for stop during countdown\r\n\tn NAME\t- Channel name, set to NAME or unset if empty\r\n\ts\t- Show output status\r\n\th or ?\t- Show this help\r\n\r\nOutput status saved automatically to EEPROM with Active or Passive commands and restored when power-up.\r\nCommands are not case sensitive. Command line are not editable.\r\n\r\n");
#define U_PROMPT uart_puts_P("\r\nSConSwitch > ");
#define U_F uart_puts_P("\r\nUART Frame Error.\r\n");
#define U_OR uart_puts_P("\r\nUART Overrun Error.\r\n");
#define U_OF uart_puts_P("\r\nBuffer overflow error.\r\n");
#define U_CMNDLONG uart_puts_P("\r\nCommand too long.\r\n");
#define U_ERR uart_puts_P("\r\nCommand error.\r\n");

# endif // _MAIN_H
