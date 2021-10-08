#include <glib.h>
#include <glib/gstdio.h>

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>

#include "configuration.h"
#include "logger.h"
#include "system_func.h"

int is_leaf(xmlNode * node)
{
  xmlNode * child = node->children;

  while(child)
  {
    if(child->type == XML_ELEMENT_NODE) return 0;

    child = child->next;
  }

  return 1;
}


void parse_xml(xmlNode * node, ServConfig* configuration)
{
	xmlNode * nodes = node->children;


    while(nodes)
    {
        if(nodes->type == XML_ELEMENT_NODE)
        {
        	if (strcmp((gchar*)nodes->name,"common") == 0)
        	{
        		CommonConf* common_conf = &configuration->common_conf;

        		g_printf("system : settings:\n");
        		if (common_conf->server_name!=NULL)
        		{
        			g_free(common_conf->server_name);
        			common_conf->server_name = NULL;
        		}
        		common_conf->server_name = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"server_name"));
        		g_printf("system : 	Server name: %s\n", common_conf->server_name);

        		if (common_conf->log_dir!=NULL)
        		{
        			g_free(common_conf->log_dir);
        			common_conf->log_dir = NULL;
        		}
        		common_conf->log_dir = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"log_dir"));
        		g_printf("system : 	log dir: %s\n", common_conf->log_dir);

        		common_conf->log_enable = atoi((gchar*)xmlGetProp(nodes,(guchar*)"log_enable"));
        		g_printf("system : 	log enable: %s\n", bool_to_str(common_conf->log_enable));

        		common_conf->log_trace = atoi((gchar*)xmlGetProp(nodes,(guchar*)"log_trace"));
        		g_printf("system : 	log_trace: %s\n", bool_to_str(common_conf->log_trace));

        		common_conf->file_size = atoi((gchar*)xmlGetProp(nodes,(guchar*)"file_size"));
        		g_printf("system : 	file_size: %d Mb\n", common_conf->file_size);

        		common_conf->save_days = atoi((gchar*)xmlGetProp(nodes,(guchar*)"save_days"));
        		g_printf("system : 	save_days: %d\n", common_conf->save_days);


        		common_conf->port = atoi((char*)xmlGetProp(nodes,(guchar*)"port"));
        		g_printf("system : 	Port: %d\n", common_conf->port);

        		if (common_conf->conn_log_dir!=NULL)
        		{
        			g_free(common_conf->conn_log_dir);
        			common_conf->conn_log_dir = NULL;
        		}
        		common_conf->conn_log_dir = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_dir"));
        		g_printf("system : 	connections_log_dir: %s\n", common_conf->conn_log_dir);

        		common_conf->conn_log_enable = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_enable"));
        		g_printf("system : 	conn_log_enable: %s\n", bool_to_str(common_conf->conn_log_enable));

        		common_conf->conn_log_trace = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_trace"));
        		g_printf("system : 	conn_log_trace: %s\n", bool_to_str(common_conf->conn_log_trace));

        		common_conf->conn_log_frames = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_frames"));
        		g_printf("system : 	conn_log_frames: %s\n", bool_to_str(common_conf->conn_log_frames));

        		common_conf->conn_log_parsing = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_parsing"));
        		g_printf("system : 	conn_log_parsing: %s\n", bool_to_str(common_conf->conn_log_parsing));

        		common_conf->conn_log_file_size = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_file_size"));
        		g_printf("system : 	conn_log_file_size: %d Mb\n", common_conf->conn_log_file_size);

        		common_conf->conn_log_save_days = atoi((gchar*)xmlGetProp(nodes,(guchar*)"connections_log_save_days"));
        		g_printf("system : 	conn_log_save_days: %d\n", common_conf->conn_log_save_days);


        		common_conf->changed = TRUE;
        	}
        	else if (strcmp((char*)nodes->name,"profiles") == 0)
        	{
        		g_printf("system : Profiles:\n");

        		configuration->profiles_conf.enable = atoi((char*)xmlGetProp(nodes,(unsigned char*)"enable"));
        		g_printf("system : 	Enable: %s\n", bool_to_str(configuration->profiles_conf.enable));


        		xmlNode * child = nodes->children;
        		while(child)
        		{
        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"profile") == 0  && configuration->profiles_conf.profiles_count < MAX_CLIENT_COUNT)
        			{
        				Profile* profile = &configuration->profiles_conf.profiles[configuration->profiles_conf.profiles_count];

        				profile->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));
        				profile->enable = atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));
        				profile->access_level = atoi((gchar*)xmlGetProp(child,(guchar*)"access_level"));

                		if (profile->name!=NULL)
                		{
                			g_free(profile->name);
                			profile->name = NULL;
                		}
                		profile->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		if (profile->guid!=NULL)
                		{
                			g_free(profile->guid);
                			profile->guid = NULL;
                		}
                		profile->guid = g_strdup((gchar*)xmlGetProp(child,(guchar*)"guid"));

                    	g_printf("system : 	Profile id=%d name=%s enable=%s id=%s access_level=%d\n",
                    			profile->id,
								profile->name,
										bool_to_str(profile->enable),
										profile->guid,
										profile->access_level);

                    	configuration->profiles_conf.profiles_count++;
        			}
        		    child = child->next;
        		}
        	}
        	else if (strcmp((char*)nodes->name,"devices") == 0)
        	{
        		g_printf("system : Devices:\n");
        		xmlNode * child = nodes->children;

        		while(child)
        		{
        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"dispencer_controller") == 0 && configuration->devices.dispencer_controller_count < MAX_DEVICE_COUNT)
        			{
        				DispencerControllerConf* device = &configuration->devices.dispencer_controllers[configuration->devices.dispencer_controller_count];

                		device->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));

                		if (device->name!=NULL)
                		{
                			g_free(device->name);
                			device->name = NULL;
                		}
                		device->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		device->port = atoi((gchar*) xmlGetProp(child,(guchar*)"port"));
                		device->enable = atoi((gchar*) xmlGetProp(child,(guchar*)"enable"));
                		device->command_timeout = atoi((gchar*) xmlGetProp(child,(guchar*)"command_timeout"));
                		device->interval = atoi((gchar*) xmlGetProp(child,(guchar*)"interval"));

                		if (device->module_name!=NULL)
                		{
                			g_free(device->module_name);
                			device->module_name = NULL;
                		}
                		device->module_name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"module_name"));

                		if (device->log_dir!=NULL)
                		{
                			g_free(device->log_dir);
                			device->log_dir = NULL;
                		}
                		device->log_dir = g_strdup((gchar*)xmlGetProp(child,(guchar*)"log_dir"));

                		device->log_enable = atoi((gchar*)xmlGetProp(child,(guchar*)"log_enable"));
                		device->log_trace = atoi((gchar*)xmlGetProp(child,(guchar*)"log_trace"));

                		device->file_size = atoi((gchar*)xmlGetProp(child,(guchar*)"file_size"));
                		device->save_days = atoi((gchar*)xmlGetProp(child,(guchar*)"save_days"));


                		device->index = configuration->devices.dispencer_controller_count;

                		g_printf("system : 	Dispencer controller %d name=%s port=%d enable=%s command_timeout=%d interval=%d module_name=%s log_dir = %s log_enable = %d log_trace = %d file_size = %d save_days = %d\n",
                				device->index,
								device->name,
								device->port,
								bool_to_str(device->enable),
								device->command_timeout,
								device->interval,
								device->module_name,
								device->log_dir,
								device->log_enable,
								device->log_trace,
								device->file_size,
								device->save_days);

                		device->thread_status = ts_Undefined;

                		xmlNode * module_settings = child->children;

                		DCLibConfig* lib_conf = &device->module_config;

                		while(module_settings)
                		{
                			if(module_settings->type == XML_ELEMENT_NODE && strcmp((gchar*)module_settings->name,"module_settings") == 0)
                			{
                        		g_printf("system : 		Module settings:\n");


                				xmlNode * child_prop = module_settings->children;

                        		while(child_prop)
                        		{
                        			if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"log") == 0)
                        			{
                        				LibLogOptions* log_options = &lib_conf->log_options;

                        				log_options->enable =  atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));

                                		if (log_options->dir!=NULL)
                                		{
                                			g_free(log_options->dir);
                                			log_options->dir = NULL;
                                		}
                                		log_options->dir = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"dir"));

                                		log_options->file_size = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"file_size"));
                                		log_options->save_days = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"save_days"));

                        				log_options->trace =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"trace"));
                        				log_options->system =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"system"));
                        				log_options->requests =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"requests"));
                        				log_options->frames =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"frames"));
                        				log_options->parsing =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"parsing"));

                                		g_printf("system : 			Log settings: enable = %d, dir = %s, trace = %d, system = %d, requests = %d, frames = %d, parsing = %d, file_size = %d, save_days = %d\n",
                                				log_options->enable, log_options->dir, log_options->trace,  log_options->system, log_options->requests, log_options->frames, log_options->parsing, log_options->file_size, log_options->save_days);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"connection") == 0)
                        			{
                        				ConnOptions* conn_options = &lib_conf->conn_options;

                        				conn_options->connection_type =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"type"));

                                		if (conn_options->port!=NULL)
                                		{
                                			g_free(conn_options->port);
                                			conn_options->port = NULL;
                                		}
                                		conn_options->port = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"port"));

                                		if (conn_options->ip_address!=NULL)
                                		{
                                			g_free(conn_options->ip_address);
                                			conn_options->ip_address = NULL;
                                		}
                                		conn_options->ip_address = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"ip_address"));

                                		conn_options->ip_port =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"ip_port"));
                                		conn_options->uart_baudrate =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_baudrate"));
                                		conn_options->uart_byte_size =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_byte_size"));
                                		conn_options->uart_parity = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"uart_parity"));
                                		conn_options->uart_stop_bits =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_stop_bits"));

                                		g_printf("system : 			Connection settings: type = %d, port = %s, ip_address = %s, ip_port = %d, uart_baudrate = %d, uart_byte_size = %d, uart_parity = %s, uart_stop_bits = %d\n",
                                				conn_options->connection_type, conn_options->port, conn_options->ip_address,  conn_options->ip_port, conn_options->uart_baudrate, conn_options->uart_byte_size,
												conn_options->uart_parity, conn_options->uart_stop_bits);

                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"timeouts") == 0)
                        			{
                        				TimeoutOptions* timeout_options = &lib_conf->timeout_options;

                        				timeout_options->t_read =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"read"));
                        				timeout_options->t_write =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"write"));

                                		g_printf("system : 			Timeout settings: read = %d, write = %d\n",
                                				timeout_options->t_read,  timeout_options->t_write);
                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"decimal_points") == 0)
                        			{
                        				DCDecimalPointOptions* dp_options = &lib_conf->decimal_point_options;

                        				dp_options->dp_price =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"price_decimal_point"));
                        				dp_options->dp_volume =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"volume_decimal_point"));
                        				dp_options->dp_amount =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"amount_decimal_point"));

                                		g_printf("system : 			decimal points settings: price = %d, volume = %d, amount = %d\n",
                                				dp_options->dp_price,  dp_options->dp_volume, dp_options->dp_amount);

                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"counters") == 0)
                        			{
                        				lib_conf->counters_enable = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			counters enable: %d\n",	lib_conf->counters_enable);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"auto_start") == 0)
                        			{
                        				lib_conf->auto_start = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			auto start: %d\n",	lib_conf->auto_start);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"auto_payment") == 0)
                        			{
                        				lib_conf->auto_payment = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			auto payment: %d\n",	lib_conf->auto_payment);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"full_tank_volume") == 0)
                        			{
                        				lib_conf->full_tank_volume = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"value"));

                                		g_printf("system : 			full tank volume: %d\n",	lib_conf->full_tank_volume);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"mapping") == 0)
                        			{
                        				xmlNode * mapping_nodes = child_prop->children;

                                		g_printf("system : 			mapping:\n");

                                		lib_conf->dispencer_count = 0;

                        				while(mapping_nodes)
                        				{
                        					if(mapping_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)mapping_nodes->name,"dispencer") == 0 && lib_conf->dispencer_count < MAX_DISP_COUNT)
                        					{
                        						DispencerConf* dispencer_conf = &lib_conf->dispencers[lib_conf->dispencer_count];

                        						dispencer_conf->num = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"num"));
                        						dispencer_conf->addr = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"addr"));

                                        		g_printf("system : 				dispencer: num = %d, addr = %d\n", dispencer_conf->num, dispencer_conf->addr);


                                        		dispencer_conf->nozzle_count = 0;

                                				xmlNode * nozzles_nodes = mapping_nodes->children;

                                				while(nozzles_nodes)
                                				{
                                					if(nozzles_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)nozzles_nodes->name,"nozzle") == 0 && dispencer_conf->nozzle_count < MAX_NOZZLE_COUNT)
                                					{
                                						NozzleConf* nozzle_conf = &dispencer_conf->nozzles[dispencer_conf->nozzle_count];

                                						nozzle_conf->num = atoi((gchar*)xmlGetProp(nozzles_nodes,(guchar*)"num"));
                                						nozzle_conf->grade = atoi((gchar*)xmlGetProp(nozzles_nodes,(guchar*)"grade"));

                                                		g_printf("system : 					nozzle: num = %d, grade = %d\n", nozzle_conf->num, nozzle_conf->grade);

                                						dispencer_conf->nozzle_count++;
                                					}

                                					nozzles_nodes = nozzles_nodes->next;
                                				}
                        						lib_conf->dispencer_count++;
                        					}

                        					mapping_nodes = mapping_nodes->next;
                        				}
                        			}
                        			child_prop = child_prop->next;
                        		}
                			}
                			module_settings = module_settings->next;
                		}
                		device->changed = TRUE;

                		configuration->devices.dispencer_controller_count++;
        			}
        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"tgs") == 0 && configuration->devices.tgs_count < MAX_DEVICE_COUNT)
        			{
        				TgsConf* device = &configuration->devices.tgs[configuration->devices.tgs_count];

                		device->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));

                		if (device->name!=NULL)
                		{
                			g_free(device->name);
                			device->name = NULL;
                		}
                		device->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		device->port = atoi((gchar*) xmlGetProp(child,(guchar*)"port"));
                		device->enable = atoi((gchar*) xmlGetProp(child,(guchar*)"enable"));
                		device->command_timeout = atoi((gchar*) xmlGetProp(child,(guchar*)"command_timeout"));
                		device->interval = atoi((gchar*) xmlGetProp(child,(guchar*)"interval"));

                		if (device->module_name!=NULL)
                		{
                			g_free(device->module_name);
                			device->module_name = NULL;
                		}
                		device->module_name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"module_name"));

                		if (device->log_dir!=NULL)
                		{
                			g_free(device->log_dir);
                			device->log_dir = NULL;
                		}
                		device->log_dir = g_strdup((gchar*)xmlGetProp(child,(guchar*)"log_dir"));

                		device->log_enable = atoi((gchar*)xmlGetProp(child,(guchar*)"log_enable"));
                		device->log_trace = atoi((gchar*)xmlGetProp(child,(guchar*)"log_trace"));

                		device->file_size = atoi((gchar*)xmlGetProp(child,(guchar*)"file_size"));
                		device->save_days = atoi((gchar*)xmlGetProp(child,(guchar*)"save_days"));

                		device->index = configuration->devices.tgs_count;

                		g_printf("system : 	Tgs %d name=%s port=%d enable=%s command_timeout=%d interval=%d module_name=%s log_dir = %s log_enable = %d log_trace = %d file_size = %d save_days = %d\n",
                				device->index,
								device->name,
								device->port,
								bool_to_str(device->enable),
								device->command_timeout,
								device->interval,
								device->module_name,
								device->log_dir,
								device->log_enable,
								device->log_trace,
								device->file_size,
								device->save_days);

                		device->thread_status = ts_Undefined;

                		xmlNode * module_settings = child->children;

                		TGSLibConfig* lib_conf = &device->module_config;

                		while(module_settings)
                		{
                			if(module_settings->type == XML_ELEMENT_NODE && strcmp((gchar*)module_settings->name,"module_settings") == 0)
                			{
                        		g_printf("system : 		Module settings:\n");


                				xmlNode * child_prop = module_settings->children;

                        		while(child_prop)
                        		{
                        			if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"log") == 0)
                        			{
                        				LibLogOptions* log_options = &lib_conf->log_options;

                        				log_options->enable =  atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));

                                		if (log_options->dir!=NULL)
                                		{
                                			g_free(log_options->dir);
                                			log_options->dir = NULL;
                                		}
                                		log_options->dir = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"dir"));

                        				log_options->trace =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"trace"));
                        				log_options->system =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"system"));
                        				log_options->requests =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"requests"));
                        				log_options->frames =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"frames"));
                        				log_options->parsing =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"parsing"));

                        				log_options->file_size =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"file_size"));
                        				log_options->save_days =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"save_days"));

                                		g_printf("system : 			Log settings: enable = %d, dir = %s, trace = %d, system = %d, requests = %d, frames = %d, parsing = %d, file_size = %d, save_days = %d\n",
                                				log_options->enable, log_options->dir, log_options->trace,  log_options->system, log_options->requests, log_options->frames, log_options->parsing, log_options->file_size, log_options->save_days);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"connection") == 0)
                        			{
                        				ConnOptions* conn_options = &lib_conf->conn_options;

                        				conn_options->connection_type =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"type"));

                                		if (conn_options->port!=NULL)
                                		{
                                			g_free(conn_options->port);
                                			conn_options->port = NULL;
                                		}
                                		conn_options->port = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"port"));

                                		if (conn_options->ip_address!=NULL)
                                		{
                                			g_free(conn_options->ip_address);
                                			conn_options->ip_address = NULL;
                                		}
                                		conn_options->ip_address = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"ip_address"));

                                		conn_options->ip_port =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"ip_port"));

                                		conn_options->uart_baudrate = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_baudrate"));
                                		conn_options->uart_byte_size = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_byte_size"));

                                		if (conn_options->uart_parity!=NULL)
                                		{
                                			g_free(conn_options->uart_parity);
                                			conn_options->uart_parity = NULL;
                                		}
                                		conn_options->uart_parity = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"uart_parity"));
                                		conn_options->uart_stop_bits = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_stop_bits"));

                                		g_printf("system : 			Connection settings: type = %d, port = %s, ip_address = %s, ip_port = %d, uart_baudrate = %d, uart_byte_size = %d, uart_parity = %s, uart_stop_bits = %d\n",
                                				conn_options->connection_type, conn_options->port, conn_options->ip_address,  conn_options->ip_port, conn_options->uart_baudrate, conn_options->uart_byte_size, conn_options->uart_parity, conn_options->uart_stop_bits);


                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"timeouts") == 0)
                        			{
                        				TimeoutOptions* timeout_options = &lib_conf->timeout_options;

                        				timeout_options->t_read =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"read"));
                        				timeout_options->t_write =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"write"));

                                		g_printf("system : 			Timeout settings: read = %d, write = %d\n",
                                				timeout_options->t_read,  timeout_options->t_write);
                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"mapping") == 0)
                        			{
                        				xmlNode * mapping_nodes = child_prop->children;

                                		g_printf("system : 			mapping:\n");

                                		lib_conf->tank_count = 0;

                        				while(mapping_nodes)
                        				{
                        					if(mapping_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)mapping_nodes->name,"tank") == 0 && lib_conf->tank_count < MAX_TANK_COUNT)
                        					{
                        						TankConf* tank_conf = &lib_conf->tanks[lib_conf->tank_count];

                        						tank_conf->num = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"num"));
                        						tank_conf->channel = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"channel"));

                                        		g_printf("system : 				tank: num = %d, channel = %d\n", tank_conf->num, tank_conf->channel);


                        						lib_conf->tank_count++;
                        					}

                        					mapping_nodes = mapping_nodes->next;
                        				}
                        			}
                        			child_prop = child_prop->next;
                        		}
                			}
                			module_settings = module_settings->next;
                		}
                		device->changed = TRUE;

                		configuration->devices.tgs_count++;
        			}

        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"price_pole_controller") == 0 && configuration->devices.price_pole_controller_count < MAX_DEVICE_COUNT)
        			{
        				PpcConf* device = &configuration->devices.price_pole_controllers[configuration->devices.price_pole_controller_count];

                		device->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));

                		if (device->name!=NULL)
                		{
                			g_free(device->name);
                			device->name = NULL;
                		}
                		device->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		device->port = atoi((gchar*) xmlGetProp(child,(guchar*)"port"));
                		device->enable = atoi((gchar*) xmlGetProp(child,(guchar*)"enable"));
                		device->command_timeout = atoi((gchar*) xmlGetProp(child,(guchar*)"command_timeout"));
                		device->interval = atoi((gchar*) xmlGetProp(child,(guchar*)"interval"));

                		if (device->module_name!=NULL)
                		{
                			g_free(device->module_name);
                			device->module_name = NULL;
                		}
                		device->module_name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"module_name"));

                		if (device->log_dir!=NULL)
                		{
                			g_free(device->log_dir);
                			device->log_dir = NULL;
                		}
                		device->log_dir = g_strdup((gchar*)xmlGetProp(child,(guchar*)"log_dir"));

                		device->log_enable = atoi((gchar*)xmlGetProp(child,(guchar*)"log_enable"));
                		device->log_trace = atoi((gchar*)xmlGetProp(child,(guchar*)"log_trace"));

                		device->file_size = atoi((gchar*)xmlGetProp(child,(guchar*)"file_size"));
                		device->save_days = atoi((gchar*)xmlGetProp(child,(guchar*)"save_days"));

                		device->index = configuration->devices.price_pole_controller_count;

                		g_printf("system : 	Price pole controller %d name=%s port=%d enable=%s command_timeout=%d interval=%d module_name=%s log_dir = %s log_enable = %d log_trace = %d file_size = %d save_days = %d\n",
                				device->index,
								device->name,
								device->port,
								bool_to_str(device->enable),
								device->command_timeout,
								device->interval,
								device->module_name,
								device->log_dir,
								device->log_enable,
								device->log_trace,
								device->file_size,
								device->save_days);

                		device->thread_status = ts_Undefined;

                		xmlNode * module_settings = child->children;

                		PPCLibConfig* lib_conf = &device->module_config;

                		while(module_settings)
                		{
                			if(module_settings->type == XML_ELEMENT_NODE && strcmp((gchar*)module_settings->name,"module_settings") == 0)
                			{
                        		g_printf("system : 		Module settings:\n");


                				xmlNode * child_prop = module_settings->children;

                        		while(child_prop)
                        		{
                        			if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"log") == 0)
                        			{
                        				LibLogOptions* log_options = &lib_conf->log_options;

                        				log_options->enable =  atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));

                                		if (log_options->dir!=NULL)
                                		{
                                			g_free(log_options->dir);
                                			log_options->dir = NULL;
                                		}
                                		log_options->dir = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"dir"));

                        				log_options->trace =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"trace"));
                        				log_options->system =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"system"));
                        				log_options->requests =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"requests"));
                        				log_options->frames =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"frames"));
                        				log_options->parsing =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"parsing"));

                        				log_options->file_size =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"file_size"));
                        				log_options->save_days =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"save_days"));

                                		g_printf("system : 			Log settings: enable = %d, dir = %s, trace = %d, system = %d, requests = %d, frames = %d, parsing = %d, file_size = %d, save_days = %d\n",
                                				log_options->enable, log_options->dir, log_options->trace,  log_options->system, log_options->requests, log_options->frames, log_options->parsing, log_options->file_size, log_options->save_days);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"connection") == 0)
                        			{
                        				ConnOptions* conn_options = &lib_conf->conn_options;

                        				conn_options->connection_type =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"type"));

                                		if (conn_options->port!=NULL)
                                		{
                                			g_free(conn_options->port);
                                			conn_options->port = NULL;
                                		}
                                		conn_options->port = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"port"));

                                		if (conn_options->ip_address!=NULL)
                                		{
                                			g_free(conn_options->ip_address);
                                			conn_options->ip_address = NULL;
                                		}
                                		conn_options->ip_address = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"ip_address"));

                                		conn_options->ip_port =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"ip_port"));

                                		conn_options->uart_baudrate = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_baudrate"));
                                		conn_options->uart_byte_size = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_byte_size"));

                                		if (conn_options->uart_parity!=NULL)
                                		{
                                			g_free(conn_options->uart_parity);
                                			conn_options->uart_parity = NULL;
                                		}
                                		conn_options->uart_parity = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"uart_parity"));
                                		conn_options->uart_stop_bits = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_stop_bits"));

                                		g_printf("system : 			Connection settings: type = %d, port = %s, ip_address = %s, ip_port = %d, uart_baudrate = %d, uart_byte_size = %d, uart_parity = %s, uart_stop_bits = %d\n",
                                				conn_options->connection_type, conn_options->port, conn_options->ip_address,  conn_options->ip_port, conn_options->uart_baudrate, conn_options->uart_byte_size, conn_options->uart_parity, conn_options->uart_stop_bits);


                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"timeouts") == 0)
                        			{
                        				TimeoutOptions* timeout_options = &lib_conf->timeout_options;

                        				timeout_options->t_read =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"read"));
                        				timeout_options->t_write =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"write"));

                                		g_printf("system : 			Timeout settings: read = %d, write = %d\n",
                                				timeout_options->t_read,  timeout_options->t_write);
                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"decimal_points") == 0)
                        			{
                        				PPCDecimalPointOptions* dp_options = &lib_conf->decimal_point_options;

                        				dp_options->dp_price =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"price_decimal_point"));

                                		g_printf("system : 			decimal points settings: price = %d\n",
                                				dp_options->dp_price);

                        			}


                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"mapping") == 0)
                        			{
                        				xmlNode * mapping_nodes = child_prop->children;

                                		g_printf("system : 			mapping:\n");

                                		lib_conf->price_pole_count = 0;

                        				while(mapping_nodes)
                        				{
                        					if(mapping_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)mapping_nodes->name,"price_pole") == 0 && lib_conf->price_pole_count < MAX_PRICE_POLE_COUNT)
                        					{
                        						PricePoleConf* price_pole_conf = &lib_conf->price_poles[lib_conf->price_pole_count];

                        						price_pole_conf->num = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"num"));
                        						price_pole_conf->grade = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"grade"));
                        						price_pole_conf->symbol_count = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"symbol_count"));

                                        		g_printf("system : 				price pole: num = %d, grade = %d, symbol count = %d\n", price_pole_conf->num, price_pole_conf->grade, price_pole_conf->symbol_count);


                        						lib_conf->price_pole_count++;
                        					}

                        					mapping_nodes = mapping_nodes->next;
                        				}
                        			}
                        			child_prop = child_prop->next;
                        		}
                			}
                			module_settings = module_settings->next;
                		}
                		device->changed = TRUE;

                		configuration->devices.price_pole_controller_count++;
        			}

        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"sensor_controller") == 0 && configuration->devices.sensor_controller_count < MAX_DEVICE_COUNT)
        			{
        				ScConf* device = &configuration->devices.sensor_controllers[configuration->devices.sensor_controller_count];

                		device->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));

                		if (device->name!=NULL)
                		{
                			g_free(device->name);
                			device->name = NULL;
                		}
                		device->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		device->port = atoi((gchar*) xmlGetProp(child,(guchar*)"port"));
                		device->enable = atoi((gchar*) xmlGetProp(child,(guchar*)"enable"));
                		device->command_timeout = atoi((gchar*) xmlGetProp(child,(guchar*)"command_timeout"));
                		device->interval = atoi((gchar*) xmlGetProp(child,(guchar*)"interval"));

                		if (device->module_name!=NULL)
                		{
                			g_free(device->module_name);
                			device->module_name = NULL;
                		}
                		device->module_name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"module_name"));

                		if (device->log_dir!=NULL)
                		{
                			g_free(device->log_dir);
                			device->log_dir = NULL;
                		}
                		device->log_dir = g_strdup((gchar*)xmlGetProp(child,(guchar*)"log_dir"));

                		device->log_enable = atoi((gchar*)xmlGetProp(child,(guchar*)"log_enable"));
                		device->log_trace = atoi((gchar*)xmlGetProp(child,(guchar*)"log_trace"));

                		device->file_size = atoi((gchar*)xmlGetProp(child,(guchar*)"file_size"));
                		device->save_days = atoi((gchar*)xmlGetProp(child,(guchar*)"save_days"));

                		device->index = configuration->devices.price_pole_controller_count;

                		g_printf("system : 	Sensor controller %d name=%s port=%d enable=%s command_timeout=%d interval=%d module_name=%s log_dir = %s log_enable = %d log_trace = %d file_size = %d save_days = %d\n",
                				device->index,
								device->name,
								device->port,
								bool_to_str(device->enable),
								device->command_timeout,
								device->interval,
								device->module_name,
								device->log_dir,
								device->log_enable,
								device->log_trace,
								device->file_size,
								device->save_days);

                		device->thread_status = ts_Undefined;

                		xmlNode * module_settings = child->children;

                		SCLibConfig* lib_conf = &device->module_config;

                		while(module_settings)
                		{
                			if(module_settings->type == XML_ELEMENT_NODE && strcmp((gchar*)module_settings->name,"module_settings") == 0)
                			{
                        		g_printf("system : 		Module settings:\n");


                				xmlNode * child_prop = module_settings->children;

                        		while(child_prop)
                        		{
                        			if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"log") == 0)
                        			{
                        				LibLogOptions* log_options = &lib_conf->log_options;

                        				log_options->enable =  atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));

                                		if (log_options->dir!=NULL)
                                		{
                                			g_free(log_options->dir);
                                			log_options->dir = NULL;
                                		}
                                		log_options->dir = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"dir"));

                        				log_options->trace =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"trace"));
                        				log_options->system =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"system"));
                        				log_options->requests =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"requests"));
                        				log_options->frames =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"frames"));
                        				log_options->parsing =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"parsing"));

                        				log_options->file_size =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"file_size"));
                        				log_options->save_days =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"save_days"));

                                		g_printf("system : 			Log settings: enable = %d, dir = %s, trace = %d, system = %d, requests = %d, frames = %d, parsing = %d, file_size = %d, save_days = %d\n",
                                				log_options->enable, log_options->dir, log_options->trace,  log_options->system, log_options->requests, log_options->frames, log_options->parsing, log_options->file_size, log_options->save_days);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"connection") == 0)
                        			{
                        				ConnOptions* conn_options = &lib_conf->conn_options;

                        				conn_options->connection_type =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"type"));

                                		if (conn_options->port!=NULL)
                                		{
                                			g_free(conn_options->port);
                                			conn_options->port = NULL;
                                		}
                                		conn_options->port = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"port"));

                                		if (conn_options->ip_address!=NULL)
                                		{
                                			g_free(conn_options->ip_address);
                                			conn_options->ip_address = NULL;
                                		}
                                		conn_options->ip_address = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"ip_address"));

                                		conn_options->ip_port =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"ip_port"));

                                		conn_options->uart_baudrate = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_baudrate"));
                                		conn_options->uart_byte_size = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_byte_size"));

                                		if (conn_options->uart_parity!=NULL)
                                		{
                                			g_free(conn_options->uart_parity);
                                			conn_options->uart_parity = NULL;
                                		}
                                		conn_options->uart_parity = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"uart_parity"));
                                		conn_options->uart_stop_bits = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_stop_bits"));

                                		g_printf("system : 			Connection settings: type = %d, port = %s, ip_address = %s, ip_port = %d, uart_baudrate = %d, uart_byte_size = %d, uart_parity = %s, uart_stop_bits = %d\n",
                                				conn_options->connection_type, conn_options->port, conn_options->ip_address,  conn_options->ip_port, conn_options->uart_baudrate, conn_options->uart_byte_size, conn_options->uart_parity, conn_options->uart_stop_bits);


                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"timeouts") == 0)
                        			{
                        				TimeoutOptions* timeout_options = &lib_conf->timeout_options;

                        				timeout_options->t_read =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"read"));
                        				timeout_options->t_write =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"write"));

                                		g_printf("system : 			Timeout settings: read = %d, write = %d\n",
                                				timeout_options->t_read,  timeout_options->t_write);
                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"mapping") == 0)
                        			{
                        				xmlNode * mapping_nodes = child_prop->children;

                                		g_printf("system : 			mapping:\n");

                                		lib_conf->sensor_count = 0;

                        				while(mapping_nodes)
                        				{
                        					if(mapping_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)mapping_nodes->name,"sensor") == 0 && lib_conf->sensor_count < MAX_SENSOR_COUNT)
                        					{
                        						SensorConf* sensor_conf = &lib_conf->sensors[lib_conf->sensor_count];

                        						sensor_conf->num = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"num"));
                        						sensor_conf->addr = atoi((gchar*)xmlGetProp(mapping_nodes,(guchar*)"addr"));

                                        		g_printf("system : 				price pole: num = %d, addr = %d\n", sensor_conf->num, sensor_conf->addr);

                                        		sensor_conf->param_count = 0;

                                				xmlNode * params_nodes = mapping_nodes->children;

                                				while(params_nodes)
                                				{
                                					if(params_nodes->type == XML_ELEMENT_NODE && strcmp((gchar*)params_nodes->name,"param") == 0 && sensor_conf->param_count < MAX_SENSOR_PARAM_COUNT)
                                					{
                                						SensorParamConf* sensor_param_conf = &sensor_conf->params[sensor_conf->param_count];

                                						sensor_param_conf->num = atoi((gchar*)xmlGetProp(params_nodes,(guchar*)"num"));
                                						sensor_param_conf->type = atoi((gchar*)xmlGetProp(params_nodes,(guchar*)"type"));

                                                		g_printf("system : 					Param: num = %d, type = %d\n", sensor_param_conf->num, sensor_param_conf->type);

                                                		sensor_conf->param_count++;
                                					}

                                					params_nodes = params_nodes->next;
                                				}

                        						lib_conf->sensor_count++;
                        					}

                        					mapping_nodes = mapping_nodes->next;
                        				}
                        			}
                        			child_prop = child_prop->next;
                        		}
                			}
                			module_settings = module_settings->next;
                		}
                		device->changed = TRUE;

                		configuration->devices.sensor_controller_count++;
        			}
        			if(child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"fiscal_register") == 0 && configuration->devices.fiscal_register_count < MAX_DEVICE_COUNT)
        			{
        				FiscalRegisterConf* device = &configuration->devices.fiscal_registers[configuration->devices.fiscal_register_count];

                		device->id = atoi((gchar*)xmlGetProp(child,(guchar*)"id"));

                		if (device->name!=NULL)
                		{
                			g_free(device->name);
                			device->name = NULL;
                		}
                		device->name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"name"));

                		device->port = atoi((gchar*) xmlGetProp(child,(guchar*)"port"));
                		device->enable = atoi((gchar*) xmlGetProp(child,(guchar*)"enable"));
                		device->command_timeout = atoi((gchar*) xmlGetProp(child,(guchar*)"command_timeout"));
                		device->interval = atoi((gchar*) xmlGetProp(child,(guchar*)"interval"));

                		if (device->module_name!=NULL)
                		{
                			g_free(device->module_name);
                			device->module_name = NULL;
                		}
                		device->module_name = g_strdup((gchar*)xmlGetProp(child,(guchar*)"module_name"));

                		if (device->log_dir!=NULL)
                		{
                			g_free(device->log_dir);
                			device->log_dir = NULL;
                		}
                		device->log_dir = g_strdup((gchar*)xmlGetProp(child,(guchar*)"log_dir"));

                		device->log_enable = atoi((gchar*)xmlGetProp(child,(guchar*)"log_enable"));
                		device->log_trace = atoi((gchar*)xmlGetProp(child,(guchar*)"log_trace"));

                		device->file_size = atoi((gchar*)xmlGetProp(child,(guchar*)"file_size"));
                		device->save_days = atoi((gchar*)xmlGetProp(child,(guchar*)"save_days"));

                		device->index = configuration->devices.price_pole_controller_count;

                		g_printf("system : 	Fiscal register %d name=%s port=%d enable=%s command_timeout=%d interval=%d module_name=%s log_dir = %s log_enable = %d log_trace = %d file_size = %d save_days = %d\n",
                				device->index,
								device->name,
								device->port,
								bool_to_str(device->enable),
								device->command_timeout,
								device->interval,
								device->module_name,
								device->log_dir,
								device->log_enable,
								device->log_trace,
								device->file_size,
								device->save_days);

                		device->thread_status = ts_Undefined;

                		xmlNode * module_settings = child->children;

                		FRLibConfig* lib_conf = &device->module_config;

                		while(module_settings)
                		{
                			if(module_settings->type == XML_ELEMENT_NODE && strcmp((gchar*)module_settings->name,"module_settings") == 0)
                			{
                        		g_printf("system : 		Module settings:\n");

                				xmlNode * child_prop = module_settings->children;

                        		while(child_prop)
                        		{
                        			if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"log") == 0)
                        			{
                        				LibLogOptions* log_options = &lib_conf->log_options;

                        				log_options->enable =  atoi((gchar*)xmlGetProp(child,(guchar*)"enable"));

                                		if (log_options->dir!=NULL)
                                		{
                                			g_free(log_options->dir);
                                			log_options->dir = NULL;
                                		}
                                		log_options->dir = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"dir"));

                        				log_options->trace =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"trace"));
                        				log_options->system =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"system"));
                        				log_options->requests =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"requests"));
                        				log_options->frames =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"frames"));
                        				log_options->parsing =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"parsing"));

                        				log_options->file_size =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"file_size"));
                        				log_options->save_days =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"save_days"));

                                		g_printf("system : 			Log settings: enable = %d, dir = %s, trace = %d, system = %d, requests = %d, frames = %d, parsing = %d, file_size = %d, save_days = %d\n",
                                				log_options->enable, log_options->dir, log_options->trace,  log_options->system, log_options->requests, log_options->frames, log_options->parsing, log_options->file_size, log_options->save_days);

                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"connection") == 0)
                        			{
                        				ConnOptions* conn_options = &lib_conf->conn_options;

                        				conn_options->connection_type =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"type"));

                                		if (conn_options->port!=NULL)
                                		{
                                			g_free(conn_options->port);
                                			conn_options->port = NULL;
                                		}
                                		conn_options->port = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"port"));

                                		if (conn_options->ip_address!=NULL)
                                		{
                                			g_free(conn_options->ip_address);
                                			conn_options->ip_address = NULL;
                                		}
                                		conn_options->ip_address = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"ip_address"));

                                		conn_options->ip_port =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"ip_port"));
                                		conn_options->uart_baudrate = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_baudrate"));
                                		conn_options->uart_byte_size = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_byte_size"));

                                		if (conn_options->uart_parity!=NULL)
                                		{
                                			g_free(conn_options->uart_parity);
                                			conn_options->uart_parity = NULL;
                                		}
                                		conn_options->uart_parity = g_strdup((gchar*)xmlGetProp(child_prop,(guchar*)"uart_parity"));
                                		conn_options->uart_stop_bits = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"uart_stop_bits"));

                                		g_printf("system : 			Connection settings: type = %d, port = %s, ip_address = %s, ip_port = %d, uart_baudrate = %d, uart_byte_size = %d, uart_parity = %s, uart_stop_bits = %d\n",
                                				conn_options->connection_type, conn_options->port, conn_options->ip_address,  conn_options->ip_port, conn_options->uart_baudrate, conn_options->uart_byte_size, conn_options->uart_parity, conn_options->uart_stop_bits);

                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"timeouts") == 0)
                        			{
                        				TimeoutOptions* timeout_options = &lib_conf->timeout_options;

                        				timeout_options->t_read =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"read"));
                        				timeout_options->t_write =  atoi((gchar*)xmlGetProp(child_prop,(guchar*)"write"));

                                		g_printf("system : 			Timeout settings: read = %d, write = %d\n",
                                				timeout_options->t_read,  timeout_options->t_write);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"protocol_type") == 0)
                        			{
                        				lib_conf->protocol_type = atoi((gchar*)xmlGetProp(child_prop,(guchar*)"value"));

                                		g_printf("system : 			protocol type: %d\n",	lib_conf->protocol_type);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"auto_drawer") == 0)
                        			{
                        				lib_conf->auto_drawer= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			auto drawer: %d\n",	lib_conf->auto_drawer);
                        			}

                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"auto_cutting") == 0)
                        			{
                        				lib_conf->auto_cutting= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			auto cutting: %d\n",	lib_conf->auto_cutting);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"cash_num") == 0)
                        			{
                        				lib_conf->cash_num= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"value"));

                                		g_printf("system : 			cash num: %d\n",	lib_conf->cash_num);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"bn_num") == 0)
                        			{
                        				lib_conf->bn_num= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"value"));

                                		g_printf("system : 			bn num: %d\n",	lib_conf->bn_num);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"bonus_num") == 0)
                        			{
                        				lib_conf->bonus_num= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"value"));

                                		g_printf("system : 			bonus num: %d\n",	lib_conf->bonus_num);
                        			}
                        			else if(child_prop->type == XML_ELEMENT_NODE && strcmp((gchar*)child_prop->name,"time_sync") == 0)
                        			{
                        				lib_conf->time_sync= atoi((gchar*)xmlGetProp(child_prop,(guchar*)"enable"));

                                		g_printf("system : 			time_sync: %d\n",	lib_conf->time_sync);
                        			}
                        			child_prop = child_prop->next;
                        		}
                			}
                			module_settings = module_settings->next;
                		}
                		device->changed = TRUE;

                		configuration->devices.fiscal_register_count++;
        			}

        		    child = child->next;
        		}
        	}


        }
        nodes = nodes->next;
    }
}

gboolean read_settings(const gchar* filename, ServConfig* configuration)
{
	xmlDoc *doc = xmlReadFile(filename, NULL, 0);

	configuration->devices.dispencer_controller_count = 0;
	configuration->profiles_conf.profiles_count = 0;


	if (doc == NULL)
	{
		fprintf(stderr,"system : could not parse %s", filename);

		return FALSE;
	}

	xmlNode *root_element = xmlDocGetRootElement(doc);

	parse_xml(root_element, configuration);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	return TRUE;
}

void add_common(xmlNodePtr root_node, CommonConf* data)
{
    xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"common");

    if (data->server_name!=NULL)
    {
    	xmlSetProp(node, (guchar*)"server_name", (guchar*)g_strdup_printf("%s",data->server_name));
    }
    else
    {
    	xmlSetProp(node, (guchar*)"server_name", (guchar*)g_strdup_printf("%s",""));
    }
	xmlSetProp(node, (guchar*)"port", (guchar*)g_strdup_printf("%d",data->port));
    if (data->log_dir!=NULL)
    {
    	xmlSetProp(node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",data->log_dir));
    }
    else
    {
    	xmlSetProp(node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",""));
    }
	xmlSetProp(node, (guchar*)"log_enable", (guchar*)g_strdup_printf("%d",data->log_enable));
	xmlSetProp(node, (guchar*)"log_trace", (guchar*)g_strdup_printf("%d",data->log_trace));

	xmlSetProp(node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",data->file_size));
	xmlSetProp(node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",data->save_days));

    if (data->conn_log_dir!=NULL)
    {
    	xmlSetProp(node, (guchar*)"connections_log_dir", (guchar*)g_strdup_printf("%s",data->conn_log_dir));
    }
    else
    {
    	xmlSetProp(node, (guchar*)"connections_log_dir", (guchar*)g_strdup_printf("%s",""));
    }
	xmlSetProp(node, (guchar*)"connections_log_enable", (guchar*)g_strdup_printf("%d",data->conn_log_enable));
	xmlSetProp(node, (guchar*)"connections_log_trace", (guchar*)g_strdup_printf("%d",data->conn_log_trace));
	xmlSetProp(node, (guchar*)"connections_log_frames", (guchar*)g_strdup_printf("%d",data->conn_log_frames));
	xmlSetProp(node, (guchar*)"connections_log_parsing", (guchar*)g_strdup_printf("%d",data->conn_log_parsing));

	xmlSetProp(node, (guchar*)"connections_log_file_size", (guchar*)g_strdup_printf("%d",data->conn_log_file_size));
	xmlSetProp(node, (guchar*)"connections_log_save_days", (guchar*)g_strdup_printf("%d",data->conn_log_save_days));

    xmlAddChild(root_node,node);
}

void add_profiles(xmlNodePtr root_node, ProfilesConf* data)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"profiles");

	xmlSetProp(node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",data->enable));

	if (data->profiles_count > 0)
	{
		for (guint8 i = 0; i < data->profiles_count; i++)
		{
			Profile* profile = &data->profiles[i];

			xmlNodePtr profile_node = xmlNewNode(NULL,BAD_CAST"profile");

			xmlSetProp(profile_node, (guchar*)"id", (guchar*)g_strdup_printf("%d",profile->id));

			if (profile->name!=NULL)
		    {
		    	xmlSetProp(profile_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",profile->name));
		    }
		    else
		    {
		    	xmlSetProp(profile_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(profile_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",profile->enable));

			if (profile->guid!=NULL)
		    {
		    	xmlSetProp(profile_node, (guchar*)"guid", (guchar*)g_strdup_printf("%s",profile->guid));
		    }
		    else
		    {
		    	xmlSetProp(profile_node, (guchar*)"guid", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(profile_node, (guchar*)"access_level", (guchar*)g_strdup_printf("%d",profile->access_level));

			xmlAddChild(node,profile_node);

		}
	}

	xmlAddChild(root_node,node);
}

void add_devices(xmlNodePtr root_node, DeviceConfs* data)
{
	xmlNodePtr node = xmlNewNode(NULL,BAD_CAST"devices");

	if (data->dispencer_controller_count > 0)
	{
		for (guint8 i = 0; i < data->dispencer_controller_count; i++)
		{
			DispencerControllerConf* dc_conf = &data->dispencer_controllers[i];

			xmlNodePtr dc_node = xmlNewNode(NULL,BAD_CAST"dispencer_controller");

			xmlSetProp(dc_node, (guchar*)"id", (guchar*)g_strdup_printf("%d",dc_conf->id));

			if (dc_conf->name!=NULL)
		    {
		    	xmlSetProp(dc_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",dc_conf->name));
		    }
		    else
		    {
		    	xmlSetProp(dc_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(dc_node, (guchar*)"port", (guchar*)g_strdup_printf("%d",dc_conf->port));
			xmlSetProp(dc_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",dc_conf->enable));
			xmlSetProp(dc_node, (guchar*)"command_timeout", (guchar*)g_strdup_printf("%d",dc_conf->command_timeout));
			xmlSetProp(dc_node, (guchar*)"interval", (guchar*)g_strdup_printf("%d",dc_conf->interval));

			if (dc_conf->module_name!=NULL)
		    {
		    	xmlSetProp(dc_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",dc_conf->module_name));
		    }
		    else
		    {
		    	xmlSetProp(dc_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",""));
		    }

			if (dc_conf->log_dir!=NULL)
		    {
		    	xmlSetProp(dc_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",dc_conf->log_dir));
		    }
		    else
		    {
		    	xmlSetProp(dc_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(dc_node, (guchar*)"log_enable", (guchar*)g_strdup_printf("%d",dc_conf->log_enable));
			xmlSetProp(dc_node, (guchar*)"log_trace", (guchar*)g_strdup_printf("%d",dc_conf->log_trace));
			xmlSetProp(dc_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",dc_conf->file_size));
			xmlSetProp(dc_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",dc_conf->save_days));

			xmlNodePtr module_settings_node = xmlNewNode(NULL,BAD_CAST"module_settings");
			DCLibConfig* module_conf = &dc_conf->module_config;

			xmlNodePtr ms_log_node = xmlNewNode(NULL,BAD_CAST"log");
			LibLogOptions* log_conf = &module_conf->log_options;
			xmlSetProp(ms_log_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",log_conf->enable));
			if (log_conf->dir!=NULL)
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",log_conf->dir));
		    }
		    else
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(ms_log_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",log_conf->file_size));
			xmlSetProp(ms_log_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",log_conf->save_days));

			xmlSetProp(ms_log_node, (guchar*)"trace", (guchar*)g_strdup_printf("%d",log_conf->trace));
			xmlSetProp(ms_log_node, (guchar*)"system", (guchar*)g_strdup_printf("%d",log_conf->system));
			xmlSetProp(ms_log_node, (guchar*)"requests", (guchar*)g_strdup_printf("%d",log_conf->requests));
			xmlSetProp(ms_log_node, (guchar*)"frames", (guchar*)g_strdup_printf("%d",log_conf->frames));
			xmlSetProp(ms_log_node, (guchar*)"parsing", (guchar*)g_strdup_printf("%d",log_conf->parsing));
			xmlAddChild(module_settings_node,ms_log_node);


			xmlNodePtr ms_conn_node = xmlNewNode(NULL,BAD_CAST"connection");
			ConnOptions* conn_conf = &module_conf->conn_options;
			xmlSetProp(ms_conn_node, (guchar*)"type", (guchar*)g_strdup_printf("%d",conn_conf->connection_type));
			if (conn_conf->port!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",conn_conf->port));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",""));
		    }
			if (conn_conf->ip_address!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",conn_conf->ip_address));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"ip_port", (guchar*)g_strdup_printf("%d",conn_conf->ip_port));
			xmlSetProp(ms_conn_node, (guchar*)"uart_baudrate", (guchar*)g_strdup_printf("%d",conn_conf->uart_baudrate));
			xmlSetProp(ms_conn_node, (guchar*)"uart_byte_size", (guchar*)g_strdup_printf("%d",conn_conf->uart_byte_size));
			if (conn_conf->uart_parity!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",conn_conf->uart_parity));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"uart_stop_bits", (guchar*)g_strdup_printf("%d",conn_conf->uart_stop_bits));


			xmlAddChild(module_settings_node,ms_conn_node);

			xmlNodePtr ms_timeouts_node = xmlNewNode(NULL,BAD_CAST"timeouts");
			TimeoutOptions* timeouts_conf = &module_conf->timeout_options;
			xmlSetProp(ms_timeouts_node, (guchar*)"read", (guchar*)g_strdup_printf("%d",timeouts_conf->t_read));
			xmlSetProp(ms_timeouts_node, (guchar*)"write", (guchar*)g_strdup_printf("%d",timeouts_conf->t_write));
			xmlAddChild(module_settings_node,ms_timeouts_node);


			xmlNodePtr ms_dp_node = xmlNewNode(NULL,BAD_CAST"decimal_points");
			DCDecimalPointOptions* dp_conf = &module_conf->decimal_point_options;
			xmlSetProp(ms_dp_node, (guchar*)"price_decimal_point", (guchar*)g_strdup_printf("%d",dp_conf->dp_price));
			xmlSetProp(ms_dp_node, (guchar*)"volume_decimal_point", (guchar*)g_strdup_printf("%d",dp_conf->dp_volume));
			xmlSetProp(ms_dp_node, (guchar*)"amount_decimal_point", (guchar*)g_strdup_printf("%d",dp_conf->dp_amount));
			xmlAddChild(module_settings_node,ms_dp_node);

			xmlNodePtr ms_cntr_node = xmlNewNode(NULL,BAD_CAST"counters");
			xmlSetProp(ms_cntr_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",module_conf->counters_enable));
			xmlAddChild(module_settings_node,ms_cntr_node);

			xmlNodePtr ms_auto_start_node = xmlNewNode(NULL,BAD_CAST"auto_start");
			xmlSetProp(ms_auto_start_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",module_conf->auto_start));
			xmlAddChild(module_settings_node,ms_auto_start_node);

			xmlNodePtr ms_auto_payment_node = xmlNewNode(NULL,BAD_CAST"auto_payment");
			xmlSetProp(ms_auto_payment_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",module_conf->auto_payment));
			xmlAddChild(module_settings_node,ms_auto_payment_node);

			xmlNodePtr ms_full_tank_volume_node = xmlNewNode(NULL,BAD_CAST"full_tank_volume");
			xmlSetProp(ms_full_tank_volume_node, (guchar*)"value", (guchar*)g_strdup_printf("%d",module_conf->full_tank_volume));
			xmlAddChild(module_settings_node,ms_full_tank_volume_node);

			xmlNodePtr ms_mapping_node = xmlNewNode(NULL,BAD_CAST"mapping");

			if (module_conf->dispencer_count > 0)
			{
				for (guint8 i = 0; i < module_conf->dispencer_count; i++)
				{
					DispencerConf* disp_conf = &module_conf->dispencers[i];

					xmlNodePtr m_disp_node = xmlNewNode(NULL,BAD_CAST"dispencer");
					xmlSetProp(m_disp_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",disp_conf->num));
					xmlSetProp(m_disp_node, (guchar*)"addr", (guchar*)g_strdup_printf("%d",disp_conf->addr));

					if (disp_conf->nozzle_count > 0)
					{
						for (guint8 j = 0; j < disp_conf->nozzle_count; j++)\
						{
							NozzleConf* nozzle_conf = &disp_conf->nozzles[j];

							xmlNodePtr d_nozzle_node = xmlNewNode(NULL,BAD_CAST"nozzle");
							xmlSetProp(d_nozzle_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",nozzle_conf->num));
							xmlSetProp(d_nozzle_node, (guchar*)"grade", (guchar*)g_strdup_printf("%d",nozzle_conf->grade));

							xmlAddChild(m_disp_node,d_nozzle_node);
						}
					}

					xmlAddChild(ms_mapping_node,m_disp_node);
				}
			}

			xmlAddChild(module_settings_node,ms_mapping_node);

			xmlAddChild(dc_node,module_settings_node);

			xmlAddChild(node,dc_node);
		}
	}

	if (data->tgs_count > 0)
	{
		for (guint8 i = 0; i < data->tgs_count; i++)
		{
			TgsConf* tgs_conf = &data->tgs[i];

			xmlNodePtr tgs_node = xmlNewNode(NULL,BAD_CAST"tgs");

			xmlSetProp(tgs_node, (guchar*)"id", (guchar*)g_strdup_printf("%d",tgs_conf->id));

			if (tgs_conf->name!=NULL)
		    {
		    	xmlSetProp(tgs_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",tgs_conf->name));
		    }
		    else
		    {
		    	xmlSetProp(tgs_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(tgs_node, (guchar*)"port", (guchar*)g_strdup_printf("%d",tgs_conf->port));
			xmlSetProp(tgs_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",tgs_conf->enable));
			xmlSetProp(tgs_node, (guchar*)"command_timeout", (guchar*)g_strdup_printf("%d",tgs_conf->command_timeout));
			xmlSetProp(tgs_node, (guchar*)"interval", (guchar*)g_strdup_printf("%d",tgs_conf->interval));

			if (tgs_conf->module_name!=NULL)
		    {
		    	xmlSetProp(tgs_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",tgs_conf->module_name));
		    }
		    else
		    {
		    	xmlSetProp(tgs_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",""));
		    }

			if (tgs_conf->log_dir!=NULL)
		    {
		    	xmlSetProp(tgs_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",tgs_conf->log_dir));
		    }
		    else
		    {
		    	xmlSetProp(tgs_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(tgs_node, (guchar*)"log_enable", (guchar*)g_strdup_printf("%d",tgs_conf->log_enable));
			xmlSetProp(tgs_node, (guchar*)"log_trace", (guchar*)g_strdup_printf("%d",tgs_conf->log_trace));
			xmlSetProp(tgs_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",tgs_conf->file_size));
			xmlSetProp(tgs_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",tgs_conf->save_days));

			xmlNodePtr module_settings_node = xmlNewNode(NULL,BAD_CAST"module_settings");
			TGSLibConfig* module_conf = &tgs_conf->module_config;

			xmlNodePtr ms_log_node = xmlNewNode(NULL,BAD_CAST"log");
			LibLogOptions* log_conf = &module_conf->log_options;
			xmlSetProp(ms_log_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",log_conf->enable));
			if (log_conf->dir!=NULL)
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",log_conf->dir));
		    }
		    else
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_log_node, (guchar*)"trace", (guchar*)g_strdup_printf("%d",log_conf->trace));
			xmlSetProp(ms_log_node, (guchar*)"system", (guchar*)g_strdup_printf("%d",log_conf->system));
			xmlSetProp(ms_log_node, (guchar*)"requests", (guchar*)g_strdup_printf("%d",log_conf->requests));
			xmlSetProp(ms_log_node, (guchar*)"frames", (guchar*)g_strdup_printf("%d",log_conf->frames));
			xmlSetProp(ms_log_node, (guchar*)"parsing", (guchar*)g_strdup_printf("%d",log_conf->parsing));
			xmlSetProp(ms_log_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",log_conf->file_size));
			xmlSetProp(ms_log_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",log_conf->save_days));
			xmlAddChild(module_settings_node,ms_log_node);


			xmlNodePtr ms_conn_node = xmlNewNode(NULL,BAD_CAST"connection");
			ConnOptions* conn_conf = &module_conf->conn_options;
			xmlSetProp(ms_conn_node, (guchar*)"type", (guchar*)g_strdup_printf("%d",conn_conf->connection_type));
			if (conn_conf->port!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",conn_conf->port));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",""));
		    }
			if (conn_conf->ip_address!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",conn_conf->ip_address));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"ip_port", (guchar*)g_strdup_printf("%d",conn_conf->ip_port));
			xmlSetProp(ms_conn_node, (guchar*)"uart_baudrate", (guchar*)g_strdup_printf("%d",conn_conf->uart_baudrate));
			xmlSetProp(ms_conn_node, (guchar*)"uart_byte_size", (guchar*)g_strdup_printf("%d",conn_conf->uart_byte_size));
			if (conn_conf->uart_parity!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",conn_conf->uart_parity));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"uart_stop_bits", (guchar*)g_strdup_printf("%d",conn_conf->uart_stop_bits));

			xmlAddChild(module_settings_node,ms_conn_node);

			xmlNodePtr ms_timeouts_node = xmlNewNode(NULL,BAD_CAST"timeouts");
			TimeoutOptions* timeouts_conf = &module_conf->timeout_options;
			xmlSetProp(ms_timeouts_node, (guchar*)"read", (guchar*)g_strdup_printf("%d",timeouts_conf->t_read));
			xmlSetProp(ms_timeouts_node, (guchar*)"write", (guchar*)g_strdup_printf("%d",timeouts_conf->t_write));
			xmlAddChild(module_settings_node,ms_timeouts_node);



			xmlNodePtr ms_mapping_node = xmlNewNode(NULL,BAD_CAST"mapping");

			if (module_conf->tank_count > 0)
			{
				for (guint8 i = 0; i < module_conf->tank_count; i++)
				{
					TankConf* tank_conf = &module_conf->tanks[i];

					xmlNodePtr m_tank_node = xmlNewNode(NULL,BAD_CAST"tank");
					xmlSetProp(m_tank_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",tank_conf->num));
					xmlSetProp(m_tank_node, (guchar*)"channel", (guchar*)g_strdup_printf("%d",tank_conf->channel));


					xmlAddChild(ms_mapping_node,m_tank_node);
				}
			}

			xmlAddChild(module_settings_node,ms_mapping_node);

			xmlAddChild(tgs_node,module_settings_node);

			xmlAddChild(node,tgs_node);
		}
	}

	if (data->price_pole_controller_count > 0)
	{
		for (guint8 i = 0; i < data->price_pole_controller_count; i++)
		{
			PpcConf* pp_conf = &data->price_pole_controllers[i];

			xmlNodePtr pp_node = xmlNewNode(NULL,BAD_CAST"price_pole_controller");

			xmlSetProp(pp_node, (guchar*)"id", (guchar*)g_strdup_printf("%d",pp_conf->id));

			if (pp_conf->name!=NULL)
		    {
		    	xmlSetProp(pp_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",pp_conf->name));
		    }
		    else
		    {
		    	xmlSetProp(pp_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(pp_node, (guchar*)"port", (guchar*)g_strdup_printf("%d",pp_conf->port));
			xmlSetProp(pp_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",pp_conf->enable));
			xmlSetProp(pp_node, (guchar*)"command_timeout", (guchar*)g_strdup_printf("%d",pp_conf->command_timeout));
			xmlSetProp(pp_node, (guchar*)"interval", (guchar*)g_strdup_printf("%d",pp_conf->interval));

			if (pp_conf->module_name!=NULL)
		    {
		    	xmlSetProp(pp_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",pp_conf->module_name));
		    }
		    else
		    {
		    	xmlSetProp(pp_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",""));
		    }

			if (pp_conf->log_dir!=NULL)
		    {
		    	xmlSetProp(pp_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",pp_conf->log_dir));
		    }
		    else
		    {
		    	xmlSetProp(pp_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(pp_node, (guchar*)"log_enable", (guchar*)g_strdup_printf("%d",pp_conf->log_enable));
			xmlSetProp(pp_node, (guchar*)"log_trace", (guchar*)g_strdup_printf("%d",pp_conf->log_trace));
			xmlSetProp(pp_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",pp_conf->file_size));
			xmlSetProp(pp_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",pp_conf->save_days));

			xmlNodePtr module_settings_node = xmlNewNode(NULL,BAD_CAST"module_settings");
			PPCLibConfig* module_conf = &pp_conf->module_config;

			xmlNodePtr ms_log_node = xmlNewNode(NULL,BAD_CAST"log");
			LibLogOptions* log_conf = &module_conf->log_options;
			xmlSetProp(ms_log_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",log_conf->enable));
			if (log_conf->dir!=NULL)
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",log_conf->dir));
		    }
		    else
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(ms_log_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",log_conf->file_size));
			xmlSetProp(ms_log_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",log_conf->save_days));

			xmlSetProp(ms_log_node, (guchar*)"trace", (guchar*)g_strdup_printf("%d",log_conf->trace));
			xmlSetProp(ms_log_node, (guchar*)"system", (guchar*)g_strdup_printf("%d",log_conf->system));
			xmlSetProp(ms_log_node, (guchar*)"requests", (guchar*)g_strdup_printf("%d",log_conf->requests));
			xmlSetProp(ms_log_node, (guchar*)"frames", (guchar*)g_strdup_printf("%d",log_conf->frames));
			xmlSetProp(ms_log_node, (guchar*)"parsing", (guchar*)g_strdup_printf("%d",log_conf->parsing));
			xmlAddChild(module_settings_node,ms_log_node);


			xmlNodePtr ms_conn_node = xmlNewNode(NULL,BAD_CAST"connection");
			ConnOptions* conn_conf = &module_conf->conn_options;
			xmlSetProp(ms_conn_node, (guchar*)"type", (guchar*)g_strdup_printf("%d",conn_conf->connection_type));
			if (conn_conf->port!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",conn_conf->port));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",""));
		    }
			if (conn_conf->ip_address!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",conn_conf->ip_address));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"ip_port", (guchar*)g_strdup_printf("%d",conn_conf->ip_port));
			xmlSetProp(ms_conn_node, (guchar*)"uart_baudrate", (guchar*)g_strdup_printf("%d",conn_conf->uart_baudrate));
			xmlSetProp(ms_conn_node, (guchar*)"uart_byte_size", (guchar*)g_strdup_printf("%d",conn_conf->uart_byte_size));
			if (conn_conf->uart_parity!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",conn_conf->uart_parity));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"uart_stop_bits", (guchar*)g_strdup_printf("%d",conn_conf->uart_stop_bits));

			xmlAddChild(module_settings_node,ms_conn_node);

			xmlNodePtr ms_timeouts_node = xmlNewNode(NULL,BAD_CAST"timeouts");
			TimeoutOptions* timeouts_conf = &module_conf->timeout_options;
			xmlSetProp(ms_timeouts_node, (guchar*)"read", (guchar*)g_strdup_printf("%d",timeouts_conf->t_read));
			xmlSetProp(ms_timeouts_node, (guchar*)"write", (guchar*)g_strdup_printf("%d",timeouts_conf->t_write));
			xmlAddChild(module_settings_node,ms_timeouts_node);


			xmlNodePtr ms_dp_node = xmlNewNode(NULL,BAD_CAST"decimal_points");
			PPCDecimalPointOptions* dp_conf = &module_conf->decimal_point_options;
			xmlSetProp(ms_dp_node, (guchar*)"price_decimal_point", (guchar*)g_strdup_printf("%d",dp_conf->dp_price));
			xmlAddChild(module_settings_node,ms_dp_node);

			xmlNodePtr ms_mapping_node = xmlNewNode(NULL,BAD_CAST"mapping");

			if (module_conf->price_pole_count > 0)
			{
				for (guint8 i = 0; i < module_conf->price_pole_count; i++)
				{
					PricePoleConf* pp_conf = &module_conf->price_poles[i];

					xmlNodePtr m_pp_node = xmlNewNode(NULL,BAD_CAST"price_pole");
					xmlSetProp(m_pp_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",pp_conf->num));
					xmlSetProp(m_pp_node, (guchar*)"grade", (guchar*)g_strdup_printf("%d",pp_conf->grade));
					xmlSetProp(m_pp_node, (guchar*)"symbol_count", (guchar*)g_strdup_printf("%d",pp_conf->symbol_count));

					xmlAddChild(ms_mapping_node,m_pp_node);
				}
			}

			xmlAddChild(module_settings_node,ms_mapping_node);

			xmlAddChild(pp_node,module_settings_node);

			xmlAddChild(node,pp_node);
		}
	}

	if (data->sensor_controller_count > 0)
	{
		for (guint8 i = 0; i < data->sensor_controller_count; i++)
		{
			ScConf* sc_conf = &data->sensor_controllers[i];

			xmlNodePtr sc_node = xmlNewNode(NULL,BAD_CAST"sensor_controller");

			xmlSetProp(sc_node, (guchar*)"id", (guchar*)g_strdup_printf("%d",sc_conf->id));

			if (sc_conf->name!=NULL)
		    {
		    	xmlSetProp(sc_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",sc_conf->name));
		    }
		    else
		    {
		    	xmlSetProp(sc_node, (guchar*)"name", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(sc_node, (guchar*)"port", (guchar*)g_strdup_printf("%d",sc_conf->port));
			xmlSetProp(sc_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",sc_conf->enable));
			xmlSetProp(sc_node, (guchar*)"command_timeout", (guchar*)g_strdup_printf("%d",sc_conf->command_timeout));
			xmlSetProp(sc_node, (guchar*)"interval", (guchar*)g_strdup_printf("%d",sc_conf->interval));

			if (sc_conf->module_name!=NULL)
		    {
		    	xmlSetProp(sc_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",sc_conf->module_name));
		    }
		    else
		    {
		    	xmlSetProp(sc_node, (guchar*)"module_name", (guchar*)g_strdup_printf("%s",""));
		    }

			if (sc_conf->log_dir!=NULL)
		    {
		    	xmlSetProp(sc_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",sc_conf->log_dir));
		    }
		    else
		    {
		    	xmlSetProp(sc_node, (guchar*)"log_dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(sc_node, (guchar*)"log_enable", (guchar*)g_strdup_printf("%d",sc_conf->log_enable));
			xmlSetProp(sc_node, (guchar*)"log_trace", (guchar*)g_strdup_printf("%d",sc_conf->log_trace));
			xmlSetProp(sc_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",sc_conf->file_size));
			xmlSetProp(sc_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",sc_conf->save_days));

			xmlNodePtr module_settings_node = xmlNewNode(NULL,BAD_CAST"module_settings");
			SCLibConfig* module_conf = &sc_conf->module_config;

			xmlNodePtr ms_log_node = xmlNewNode(NULL,BAD_CAST"log");
			LibLogOptions* log_conf = &module_conf->log_options;
			xmlSetProp(ms_log_node, (guchar*)"enable", (guchar*)g_strdup_printf("%d",log_conf->enable));
			if (log_conf->dir!=NULL)
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",log_conf->dir));
		    }
		    else
		    {
		    	xmlSetProp(ms_log_node, (guchar*)"dir", (guchar*)g_strdup_printf("%s",""));
		    }

			xmlSetProp(ms_log_node, (guchar*)"file_size", (guchar*)g_strdup_printf("%d",log_conf->file_size));
			xmlSetProp(ms_log_node, (guchar*)"save_days", (guchar*)g_strdup_printf("%d",log_conf->save_days));

			xmlSetProp(ms_log_node, (guchar*)"trace", (guchar*)g_strdup_printf("%d",log_conf->trace));
			xmlSetProp(ms_log_node, (guchar*)"system", (guchar*)g_strdup_printf("%d",log_conf->system));
			xmlSetProp(ms_log_node, (guchar*)"requests", (guchar*)g_strdup_printf("%d",log_conf->requests));
			xmlSetProp(ms_log_node, (guchar*)"frames", (guchar*)g_strdup_printf("%d",log_conf->frames));
			xmlSetProp(ms_log_node, (guchar*)"parsing", (guchar*)g_strdup_printf("%d",log_conf->parsing));
			xmlAddChild(module_settings_node,ms_log_node);


			xmlNodePtr ms_conn_node = xmlNewNode(NULL,BAD_CAST"connection");
			ConnOptions* conn_conf = &module_conf->conn_options;
			xmlSetProp(ms_conn_node, (guchar*)"type", (guchar*)g_strdup_printf("%d",conn_conf->connection_type));
			if (conn_conf->port!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",conn_conf->port));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"port", (guchar*)g_strdup_printf("%s",""));
		    }
			if (conn_conf->ip_address!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",conn_conf->ip_address));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"ip_address", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"ip_port", (guchar*)g_strdup_printf("%d",conn_conf->ip_port));
			xmlSetProp(ms_conn_node, (guchar*)"uart_baudrate", (guchar*)g_strdup_printf("%d",conn_conf->uart_baudrate));
			xmlSetProp(ms_conn_node, (guchar*)"uart_byte_size", (guchar*)g_strdup_printf("%d",conn_conf->uart_byte_size));
			if (conn_conf->uart_parity!=NULL)
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",conn_conf->uart_parity));
		    }
		    else
		    {
		    	xmlSetProp(ms_conn_node, (guchar*)"uart_parity", (guchar*)g_strdup_printf("%s",""));
		    }
			xmlSetProp(ms_conn_node, (guchar*)"uart_stop_bits", (guchar*)g_strdup_printf("%d",conn_conf->uart_stop_bits));

			xmlAddChild(module_settings_node,ms_conn_node);

			xmlNodePtr ms_timeouts_node = xmlNewNode(NULL,BAD_CAST"timeouts");
			TimeoutOptions* timeouts_conf = &module_conf->timeout_options;
			xmlSetProp(ms_timeouts_node, (guchar*)"read", (guchar*)g_strdup_printf("%d",timeouts_conf->t_read));
			xmlSetProp(ms_timeouts_node, (guchar*)"write", (guchar*)g_strdup_printf("%d",timeouts_conf->t_write));
			xmlAddChild(module_settings_node,ms_timeouts_node);

			xmlNodePtr ms_mapping_node = xmlNewNode(NULL,BAD_CAST"mapping");

			if (module_conf->sensor_count > 0)
			{
				for (guint8 i = 0; i < module_conf->sensor_count; i++)
				{
					SensorConf* sensor_conf = &module_conf->sensors[i];

					xmlNodePtr m_sensor_node = xmlNewNode(NULL,BAD_CAST"sensor");
					xmlSetProp(m_sensor_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",sensor_conf->num));
					xmlSetProp(m_sensor_node, (guchar*)"addr", (guchar*)g_strdup_printf("%d",sensor_conf->addr));

					if (sensor_conf->param_count > 0)
					{
						for (guint8 j = 0; j < sensor_conf->param_count; j++)\
						{
							SensorParamConf* sensor_param_conf = &sensor_conf->params[j];

							xmlNodePtr sensor_param_node = xmlNewNode(NULL,BAD_CAST"param");
							xmlSetProp(sensor_param_node, (guchar*)"num", (guchar*)g_strdup_printf("%d",sensor_param_conf->num));
							xmlSetProp(sensor_param_node, (guchar*)"type", (guchar*)g_strdup_printf("%d",sensor_param_conf->type));

							xmlAddChild(m_sensor_node,sensor_param_node);
						}
					}

					xmlAddChild(ms_mapping_node,m_sensor_node);
				}
			}

			xmlAddChild(module_settings_node,ms_mapping_node);

			xmlAddChild(sc_node,module_settings_node);

			xmlAddChild(node,sc_node);
		}
	}



	xmlAddChild(root_node,node);
}

gboolean write_settings(const gchar* filename, ServConfig* configuration)
{

    xmlDocPtr doc = xmlNewDoc(BAD_CAST XML_DEFAULT_VERSION);
    if (doc == NULL)
    {
        printf("Error creating the xml document\n");
        return FALSE;
    }

    xmlNodePtr root_node = xmlNewDocNode(doc, NULL, BAD_CAST "root", NULL);
    if (root_node == NULL)
    {
        printf("Error creating the xml root node\n");
        return FALSE;
    }

    xmlDocSetRootElement(doc, root_node);

    add_common(root_node, &configuration->common_conf);
    add_profiles(root_node, &configuration->profiles_conf);
    add_devices(root_node, &configuration->devices);

    xmlSaveFormatFile(filename, doc, 1);

    xmlFreeDoc(doc);

	return TRUE;
}
