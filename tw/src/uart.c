#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "logger.h"
#include "tw.h"

struct termios options;


gint open_uart(gchar* port_name, FILE** log, LogOptions log_options, GMutex* log_mutex, const gchar* prefix)
{
	gint result;

	result = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (result == -1)
	{
		add_log(log, log_mutex, prefix, TRUE, TRUE, log_options.trace, log_options.system, "Port %s Unable to open", port_name);
	}
	else
	{
		fcntl(result, F_SETFL, 0);
		add_log(log, log_mutex, prefix, TRUE, TRUE, log_options.trace, log_options.system, "Port %s successfully opened", port_name);

	}

	return(result);
}

void set_settings_uart(gint port_descriptor, guint32 timeout, FILE** log, LogOptions log_options, GMutex* log_mutex, const gchar* prefix)
{
	if (tcgetattr(port_descriptor, &options) == -1)
	{
		add_log(log, log_mutex, prefix, TRUE, TRUE,log_options.trace, log_options.system, "Error getting com-port settings!");
	}
	else
	{

		cfsetspeed(&options, B9600);

		options.c_cflag &= ~CSTOPB;				//	один стоповый бит
		options.c_cflag &= ~CSIZE;				//	Размер символа
		options.c_cflag |= CS8;
		options.c_cflag |= PARENB;
		options.c_cflag |= PARODD;

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
			add_log(log, log_mutex, prefix, TRUE, TRUE, log_options.trace, log_options.system,  "Error stored com-port settings!");
		}
		else
		{
			add_log(log, log_mutex, prefix, TRUE, TRUE, log_options.trace, log_options.system,  "Com-port settings stored");
		}
	}

}

void close_uart(gint port_descriptor)
{
	close(port_descriptor);
}

