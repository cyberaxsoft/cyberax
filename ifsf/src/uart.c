#include <glib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "driver.h"
#include "logger.h"
#include "ifsf.h"


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

void close_uart(gint port_descriptor)
{
	close(port_descriptor);
}

