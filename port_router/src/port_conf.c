#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlwriter.h>
#include <string.h>
#include <hidapi.h>


#include "logger.h"
#include "port_router.h"
#include "port_func.h"

guint8 is_leaf(xmlNode * node)
{
  xmlNode * child = node->children;

  while(child)
  {
    if(child->type == XML_ELEMENT_NODE) return 0;

    child = child->next;
  }

  return 1;
}

void parse_xml(xmlNode * node, PortRouterConfig* configuration)
{
	xmlNode * nodes = node->children;

    while(nodes)
    {
        if(nodes->type == XML_ELEMENT_NODE)
        {
        	if (strcmp((gchar*)nodes->name,"log") == 0)
        	{
        		g_printf("Logging settings:\n");
        		configuration->log_enable = atoi((gchar*)xmlGetProp(nodes,(guchar*)"enable"));
        		g_printf("	Enable: %s\n", bool_to_str(configuration->log_enable));

        		configuration->log_options.trace = atoi((gchar*)xmlGetProp(nodes,(guchar*)"trace"));
        		g_printf(" 	Trace: %s\n", bool_to_str(configuration->log_options.trace));


           		configuration->log_options.file_size = atoi((gchar*)xmlGetProp(nodes,(guchar*)"file_size"));
            		g_printf(" 	File size: %d\n", configuration->log_options.file_size);

          		configuration->log_options.save_days = atoi((gchar*)xmlGetProp(nodes,(guchar*)"save_days"));
               		g_printf(" 	Save days: %d\n", configuration->log_options.save_days);

        		if (configuration->log_dir!=NULL)
        		{
        			g_free(configuration->log_dir);
        			configuration->log_dir = NULL;
        		}
        		configuration->log_dir = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"dir"));
        		g_printf("	Directory: %s\n", configuration->log_dir);
        	}

        	else if (strcmp((char*)nodes->name,"device") == 0 && configuration->device_count < MAX_DEVICE_COUNT)
        	{
        		UsbDevice* device = &configuration->devices[configuration->device_count];

        		device->vendor_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"vendor_id"));
        		device->product_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"product_id"));
        		device->idc_id = atoi((gchar*)xmlGetProp(nodes,(guchar*)"idc_id"));

        		device->is_connected = FALSE;
        		device->stoping = FALSE;
        		device->handle = NULL;
        		device->usb_device_thread = NULL;
        		device->stage = uds_Undefined;

        		if (device->serial_number!=NULL)
        		{
        			g_free(device->serial_number);
        			device->serial_number = NULL;
        		}
        		device->serial_number = g_strdup((gchar*)xmlGetProp(nodes,(guchar*)"serial_number"));

        		xmlNode * child = nodes->children;
        		device->port_count = 0;

        		while(child)
        		{
        			if (child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"port") == 0 && device->port_count < MAX_PORT_COUNT)
        			{
        				UsbDevicePort* port = &configuration->devices[configuration->device_count].ports[configuration->devices[configuration->device_count].port_count];

        				port->interface_id = atoi((gchar*)xmlGetProp(child,(guchar*)"interface_id"));
        				port->num = atoi((gchar*)xmlGetProp(child,(guchar*)"num"));
        				port->ip_port = atoi((gchar*)xmlGetProp(child,(guchar*)"ip_port"));
        				port->max_client_count = atoi((gchar*)xmlGetProp(child,(guchar*)"max_client_count"));

        				if (port->interface_id == PORT_TRANSFER_ID)
        				{
        					port->baudrate = atoi((gchar*)xmlGetProp(child,(guchar*)"baudrate"));
        					port->byte_size = atoi((gchar*)xmlGetProp(child,(guchar*)"byte_size"));

        					gchar*  parity = g_strdup((gchar*)xmlGetProp(child,(guchar*)"parity"));

        					if (strstr(parity,"none")!=NULL)
        					{
        						port->parity = 0;
        					}
        					else if (strstr(parity,"even")!=NULL)
        					{
        						port->parity = 1;
        					}
        					else if (strstr(parity,"odd")!=NULL)
        					{
        						port->parity = 2;
        					}

        					g_free(parity);

        					port->stop_bits = atoi((gchar*)xmlGetProp(child,(guchar*)"stop_bits"));

        					port->duplex = atoi((gchar*)xmlGetProp(child,(guchar*)"duplex"));
        					port->debug = atoi((gchar*)xmlGetProp(child,(guchar*)"debug"));
        					port->passtrough = atoi((gchar*)xmlGetProp(child,(guchar*)"passtrough"));
        					port->invert_rx = atoi((gchar*)xmlGetProp(child,(guchar*)"invert_rx"));
        					port->invert_tx = atoi((gchar*)xmlGetProp(child,(guchar*)"invert_tx"));
        					port->req_timeout = atoi((gchar*)xmlGetProp(child,(guchar*)"req_timeout"));
        					port->recv_timeout = atoi((gchar*)xmlGetProp(child,(guchar*)"recv_timeout"));
        					port->application = atoi((gchar*)xmlGetProp(child,(guchar*)"application"));
        				}

        				port->serv_thread = NULL;
        				port->sock = -1;

        				for (guint8 i = 0; i < MAX_CLIENT_COUNT; i++)
        				{
            				port->client_socks[i] = cs_Free;
        				}

        				configuration->devices[configuration->device_count].port_count++;
        			}
        		    child = child->next;
        		}
            	configuration->device_count++;

        		g_printf("Device: vendor_id = %04X  product_id = %04X serial_number = %s\n", device->vendor_id,device->product_id, device->serial_number );
        	}

        	else if (strcmp((char*)nodes->name,"mult_device") == 0 && configuration->mult_device_count < MAX_DEVICE_COUNT)
        	{
        		MultDevice* device = &configuration->mult_devices[configuration->mult_device_count];

        		device->ip_port = atoi((gchar*)xmlGetProp(nodes,(guchar*)"ip_port"));
        		device->max_client_count = atoi((gchar*)xmlGetProp(nodes,(guchar*)"max_client_count"));

        		device->serv_thread = NULL;
        		device->sock = -1;

				for (guint8 i = 0; i < MAX_CLIENT_COUNT; i++)
				{
					device->client_socks[i] = -1;
				}

				g_mutex_init(&device->mutex);

        		xmlNode * child = nodes->children;
        		device->units.unit_count = 0;

        		while(child)
        		{
        			if (child->type == XML_ELEMENT_NODE && strcmp((gchar*)child->name,"unit") == 0 && device->units.unit_count < MAX_PORT_COUNT)
        			{
        				MultDeviceUnit* unit = &device->units.units[device->units.unit_count];

                		if (unit->serial_number!=NULL)
                		{
                			g_free(unit->serial_number);
                			unit->serial_number = NULL;
                		}
                		unit->serial_number = g_strdup((gchar*)xmlGetProp(child,(guchar*)"serial_number"));

                		unit->num = atoi((gchar*)xmlGetProp(child,(guchar*)"num"));
                		unit->idc_id = atoi((gchar*)xmlGetProp(child,(guchar*)"idc_id"));

                		unit->device_index = -1;

        				device->units.unit_count++;

        			}
        			child = child->next;
        		}




            	configuration->mult_device_count++;

        		g_printf("Mult Device: port %d\n", device->ip_port );

        	}

        }
        nodes = nodes->next;
    }
}

gboolean read_settings(const gchar* filename, PortRouterConfig* configuration)
{
	xmlDoc *doc = xmlReadFile(filename, NULL, 0);

	configuration->device_count = 0;

	if (doc == NULL)
	{
		fprintf(stderr,"Could not parse %s", filename);

		return FALSE;
	}

	xmlNode *root_element = xmlDocGetRootElement(doc);

	parse_xml(root_element, configuration);

	xmlFreeDoc(doc);

	xmlCleanupParser();

	return TRUE;
}
