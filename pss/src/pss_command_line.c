#include <glib.h>
#include <glib/gstdio.h>

gboolean parse_command_line(gint argc, gchar *argv[], gchar** log_dir, gchar** config_filename,  gchar** guid, gchar** ip_address, gboolean* log_enable, gboolean* log_trace,
							 guint32* ip_port, guint32* sensor_num, guint32* sensor_param_num, guint32* file_size, guint32* save_days)
{
	gboolean result = FALSE;

	gboolean log_dir_param_found = FALSE;
	gboolean config_filename_param_found = FALSE;
	gboolean log_enable_param_found = FALSE;
	gboolean log_trace_param_found = FALSE;
	gboolean ip_address_param_found = FALSE;
	gboolean ip_port_param_found = FALSE;
	gboolean sensor_num_param_found = FALSE;
	gboolean sensor_param_num_param_found = FALSE;
	gboolean guid_param_found = FALSE;
	gboolean file_size_param_found = FALSE;
	gboolean save_days_param_found = FALSE;


	for( guint8 i = 0 ; i < argc; i++)
	{
    	if (strstr(argv[i],"--logdir:")!=NULL)
    	{
    		*log_dir = malloc( strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1));
    		memcpy(*log_dir, strchr(argv[i], ':') + 1, strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1) );
        	g_printf("Parameter logdir found: [%s]\n", *log_dir);
        	log_dir_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--configfilename:")!=NULL)
    	{
    		*config_filename = malloc( strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1));
    		memcpy(*config_filename, strchr(argv[i], ':') + 1, strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1) );
        	g_printf("Parameter configfilename found: [%s]\n", *config_filename);
        	config_filename_param_found = TRUE;
    	}

    	else if (strstr(argv[i],"--logenable:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--logenable:") + strlen("--logenable:"),"%d", log_enable);
        	g_printf("Parameter logenable found: [%d]\n", *log_enable);
        	log_enable_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--logtrace:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--logtrace:") + strlen("--logtrace:"),"%d", log_trace);
        	g_printf("Parameter logtrace found: [%d]\n", *log_trace);
        	log_trace_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--ipaddress:")!=NULL)
    	{
    		*ip_address = malloc( strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1));
    		memcpy(*ip_address, strchr(argv[i], ':') + 1, strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1) );
        	g_printf("Parameter ipaddress found: [%s]\n", *ip_address);
        	ip_address_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--ipport:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--ipport:") + strlen("--ipport:"),"%d", ip_port);
        	g_printf("Parameter ipport found: [%d]\n", *ip_port);
        	ip_port_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--sensor_num:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--sensor_num:") + strlen("--sensor_num:"),"%d",sensor_num);
        	g_printf("Parameter sensor_num found: [%d]\n", *sensor_num);
        	sensor_num_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--sensor_param_num:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--sensor_param_num:") + strlen("--sensor_param_num:"),"%d", sensor_param_num);
        	g_printf("Parameter sensor_param_num found: [%d]\n", *sensor_param_num);
        	sensor_param_num_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--guid:")!=NULL)
    	{
    		*guid = malloc( strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1));
    		memcpy(*guid, strchr(argv[i], ':') + 1, strlen(argv[i]) - (strchr(argv[i], ':') - argv[i] + 1) );
        	g_printf("Parameter guid found: [%s]\n", *guid);
        	guid_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--file_size:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--file_size:") + strlen("--file_size:"),"%d", file_size);
        	g_printf("Parameter file size found: [%d]\n", *file_size);
        	file_size_param_found = TRUE;
    	}
    	else if (strstr(argv[i],"--save_days:")!=NULL)
    	{
    		sscanf(strstr(argv[i],"--save_days:") + strlen("--save_days:"),"%d", save_days);
        	g_printf("Parameter sensor_param_num found: [%d]\n", *save_days);
        	sensor_param_num_param_found = TRUE;
        	save_days_param_found = TRUE;
    	}

    	g_printf("log_enable = %d\n", *log_enable);

	}




	if(!log_dir_param_found)
	{
	    g_printf("Parameter logdir not found! Exiting...\n");
	    return result;
	}
	if (!config_filename_param_found)
	{
	    g_printf("Parameter configfilename not found! Exiting...\n");
	    return result;
	}
	if(!log_enable_param_found)
	{
	    g_printf("Parameter logenable not found! Set default value is TRUE\n");
	    *log_enable = TRUE;
	}
	if(!log_trace_param_found)
	{
	    g_printf("Parameter logtrace not found! Set default value is TRUE\n");
	    *log_trace = TRUE;
	}
	if(!ip_address_param_found)
	{
	    g_printf("Parameter ipaddress not found! Exiting...\n");
	    return result;
	}
	if(!ip_port_param_found)
	{
	    g_printf("Parameter ipport not found! Exiting...\n");
	    return result;
	}
	if(!guid_param_found)
	{
	    g_printf("Parameter guid not found! Exiting...\n");
	    return result;
	}
	if (!sensor_num_param_found)
	{
		*sensor_num = 0;
	}
	if (!sensor_param_num_param_found)
	{
		*sensor_param_num = 0;
	}

	if (!file_size_param_found)
	{
		*file_size = 10;
	}
	if (!save_days_param_found)
	{
		*save_days = 10;
	}

	result = TRUE;



	return result;
}
