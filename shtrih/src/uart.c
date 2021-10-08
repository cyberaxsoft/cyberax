#include <glib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "driver.h"
#include "logger.h"
#include "shtrih.h"

struct termios options;


gint open_uart(gchar* port_name, gboolean trace_log, gboolean system_log)
{
	gint result;

	result = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (result == -1)
	{
		add_log(TRUE, TRUE, trace_log, system_log, "Port %s Unable to open", port_name);
	}
	else
	{
		fcntl(result, F_SETFL, 0);
		add_log(TRUE, TRUE, trace_log, system_log, "Port %s successfully opened", port_name);

	}

	return(result);
}

speed_t return_baudrate(guint32  value)
{
	speed_t result = B9600;

	switch (value)
	{
		case 0: result =  B0; break;
		case 50: result = B50; break;
		case 75: result = B75; break;
		case 110: result = B110; break;
		case 134: result = B134; break;
		case 150: result = B150; break;
		case 200: result = B200; break;
		case 300: result = B300; break;
		case 600: result = B600; break;
		case 1200: result = B1200; break;
		case 1800: result = B1800; break;
		case 2400: result = B2400; break;
		case 4800: result = B4800; break;
		case 9600: result = B9600; break;
		case 19200: result = B19200; break;
		case 38400: result = B38400; break;
		case 57600: result = B57600; break;
		case 115200: result = B115200; break;
		case 230400: result = B230400; break;
		case 460800: result = B460800; break;
		case 500000: result = B500000; break;
		case 576000: result = B576000; break;
		case 921600: result = B921600; break;
		case 1000000: result = B1000000; break;
	}

	return result;
}

void set_settings_uart(gint port_descriptor, guint32 uart_baudrate, guint8 uart_byte_size, gchar* uart_parity,  guint8 uart_stop_bits, guint32 timeout, gboolean trace_log, gboolean system_log)
{
	if (tcgetattr(port_descriptor, &options) == -1)
	{
		add_log(TRUE, TRUE,trace_log, system_log, "Error getting com-port settings!");
	}
	else
	{

		cfsetspeed(&options, return_baudrate(uart_baudrate));

		if (strstr(uart_parity, "none")!=NULL)
		{
			options.c_cflag &= ~PARENB;
		}
		else if (strstr(uart_parity, "even")!=NULL)
		{
			options.c_cflag |= PARENB;
			options.c_cflag &= ~PARODD;
		}
		else if(strstr(uart_parity, "odd")!=NULL)
		{
			options.c_cflag |= PARENB;
			options.c_cflag |= PARODD;
		}

		switch(uart_stop_bits)
		{
			case 1: options.c_cflag &= ~CSTOPB;	break;
			case 2: options.c_cflag |= CSTOPB; break;
		}


		options.c_cflag &= ~CSIZE;
		switch (uart_byte_size)
		{
			case 7: options.c_cflag |= CS7; break;
			case 8: options.c_cflag |= CS8; break;
		}

		//------------------------------------------------------- Adjust Other Options

		options.c_cflag &= ~CRTSCTS;			//	Disable RTS/CTS (hardware) flow control
		options.c_cflag |= (
				CLOCAL |						//	Локальная линия, иначе - модемное соединение с набором номера
				CREAD							//	Разрешить прием
				);

		//------------------------------------------------------- Choose Raw Input and Output

		options.c_iflag &= ~(					//	!!! НЕ !!!
			IGNBRK |						//	Игнорировать разрыв линии
			BRKINT |						//	Посылать сигнал прерывания при разрыве линии
			PARMRK |						//	Отмечать ошибки четности
			ISTRIP |						//	Срезать старший (восьмой) бит у символов
			INLCR |							//	Преобразовывать при вводе NL в CR
			IGNCR |							//	Игнорировать CR
			ICRNL |							//	Преобразовывать при вводе CR в NL
			IXON |							//	Разрешить старт/стоповое управление выводом
			IXOFF							//	Разрешить старт/стоповое управление вводом
			);

		options.c_lflag &= ~(					//	!!! НЕ !!!
			ECHO |							//	Разрешить эхо
			ECHONL |						//	Выдавать эхо на NL
			ICANON |						//	Канонический ввод (обработка забоя и стирания строки)
			ISIG |							//	Разрешить посылку сигналов
			IEXTEN							//	Разрешить расширенные (определенные реализацией) функции
			);

		options.c_oflag &= ~OPOST;				//	Постобработка вывода

		options.c_cc[VMIN]  = 0;            // read doesn't block
		options.c_cc[VTIME] =  timeout / 100;

		if (tcsetattr(port_descriptor, TCSANOW, &options) == -1)
		{
			add_log(TRUE, TRUE, trace_log, system_log,  "Error stored com-port settings!");
		}
		else
		{
			add_log(TRUE, TRUE, trace_log, system_log,  "Com-port settings stored");
		}
	}

}

void close_uart(gint port_descriptor)
{
	close(port_descriptor);
}

