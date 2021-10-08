#include <glib.h>
#include <stdio.h>
#include <math.h>
#include <sys/time.h>

#include "driver.h"
#include "logger.h"
#include "tokheim.h"
#include "config.h"
#include "driver_state.h"

guint64 get_date_time(void)
{
	struct timeval tval;

	gettimeofday(&tval, NULL);

	return tval.tv_sec * 1000 + (tval.tv_usec / 1000);
}


guint32 packed_bcd_to_bin(guint8* buff, guint8 size)
{
	guint32 result = 0;

	buff += (size - 1);
	size *= 2;

	while (size--)
	{
		result *= 10;

		if(size & 0x01)
		{
			result += (*(buff) >> 4);
		}
		else
		{
			result += (*(buff) & 0x0F);
			buff--;
		}
	}

	return result;
}

guint8 set_reply_aux_length(guint8 disp_index, TokheimAuxCommand command)
{
	switch(command)
	{
		case tac_ActivatedHose_req:
			return safe_set_reply_length(ACTIVATED_HOSE_ID_REPLY_LENGTH);

		case tac_DeactivatedHose_req:
			return safe_set_reply_length(DEACTIVATED_HOSE_ID_REPLY_LENGTH);

		case tac_SendCashPrices_req:
			return safe_set_reply_length(SEND_CASH_PRICES_REPLY_LENGTH);

	}
	return 0;
}

guint8 set_reply_length(guint8 disp_index, TokheimCommand command)
{
	//guint8 fuelling_point_id = safe_get_fuelling_point_id(disp_index);

	switch(command)
	{
		case tc_FuellingPointId_req:
			return safe_set_reply_length(FUELLING_POINT_ID_REPLY_LENGTH);

		case tc_AuxFuellingPointId_req:
			return safe_set_reply_length(FUELLING_POINT_ID_REPLY_LENGTH);
			break;

		case tc_FuellingPointDisplayData_req:
			return safe_set_reply_length(FUELLING_POINT_DISPLAY_DATA_REPLY_LENGTH); //TODO +1, +2, +3

		case tc_FuellingPointStatus_req:
			return safe_set_reply_length(FUELLING_POINT_STATUS_REPLY_LENGTH);

		case tc_SuspendFuellingPoint:
			return safe_set_reply_length(SUSPEND_FUELLING_POINT_REPLY_LENGTH);

		case tc_ResumeFuellingPoint:
			return safe_set_reply_length(RESUME_FUELLING_POINT_REPLY_LENGTH);

		case tc_AuthorizeFuellingPoint:
			return safe_set_reply_length(AUTHORIZE_FUELLING_POINT_REPLY_LENGTH);

		case tc_AuthorizeFuellingPoint2:
			return safe_set_reply_length(AUTHORIZE_FUELLING_POINT_REPLY_LENGTH);

		case tc_SendDataForFuellingPoint:
			return safe_set_reply_length(SEND_DATA_FOR_FUELLING_POINT_REPLY_LENGTH);

		case tc_ResetFuellingPoint:
			return safe_set_reply_length(RESET_FUELLING_POINT_REPLY_LENGTH);

		case tc_FuellingPointTotals_req:
//			if (safe_get_nozzle_count(disp_index) == 1)
//			{
//				return safe_set_reply_length(FUELLING_POINT_SINGLE_TOTALS_REPLY_LENGTH);
//			}
//			else
//			{
				return safe_set_reply_length(FUELLING_POINT_MULT_TOTALS_REPLY_LENGTH);
//			}
			break;

		case tc_SetFuellingPointDisplayControl:
			return safe_set_reply_length(SET_FUELLING_POINT_DISPLAY_CONTROL_REPLY_LENGTH);
			break;

	}

	return 0;
}

guint32 mult_price_litres(guint32 price, guint32 litres, guint8 litres_decimal_point)
{
	guint32 result = price * litres;

	while (litres_decimal_point--)
	{
		guint32 reminder = result % 10;

		result /= 10;

		if ((litres_decimal_point == 0) && (reminder >=5))
		{
			result++;
		}

	}

	return result;

}

guint32 div_amount_price(guint32 price, guint32 amount, guint8 litres_decimal_point)
{
	gfloat litres = (gfloat)amount / (gfloat)price;

	guint32 result = (guint32)(litres * pow(10, litres_decimal_point));

	return result;

}


guint8 add_byte_to_buffer(guint8* buffer, guint8 value)
{
	guint8 result = 0;

	buffer[result++] = value;
	buffer[result++] = value ^ 0xFF;

	return result;
}

guint8 add_value_to_buffer(guint8* buffer, guint32 value, guint8 size)
{
	guint8 result = 0;

	guint8 byte = 0;

	size *= 2;

	while(size--)
	{
		if (size & 0x01)
		{
			byte = value % 10;
		}
		else
		{
			byte |= (value % 10) << 4;

			result += add_byte_to_buffer(&buffer[result], byte);
		}

		value /= 10;
	}

	return result;
}

guint8 add_reverse_value_to_buffer(guint8* buffer, guint32 value, guint8 size)
{
	guint8 result = 0;

	guint8 byte = 0;

	size *= 2;

	while(size--)
	{
		if (size & 0x01)
		{
			byte = value / (guint32)pow(10, size);

		}
		else
		{
			byte = (byte << 4) | (value / (guint32)pow(10, size));

			result += add_byte_to_buffer(&buffer[result], byte);
		}

		value %= (guint32)pow(10, size);
	}

	return result;
}

void get_error_description(guint8 error_code, guchar* buffer)
{
	memset(buffer, 0x00, DRIVER_DESCRIPTION_LENGTH);

	switch (error_code)
	{
		default: memcpy(buffer, "Undefined error", strlen("Undefined error")); break;
	}
}

const gchar* ds_to_str(DispencerState state)
{
	switch(state)
	{
		case ds_NotInitialize: return "NotInitialize";
		case ds_Busy: return "Busy";
		case ds_Free: return "Free";
		case ds_NozzleOut: return "NozzleOut";
		case ds_Filling: return "Filling";
		case ds_Stopped: return "Stopped";
		case ds_Finish: return "Finish";
		case ds_ConnectionError: return "ConnectionError";
		default: return "Undefined";
	}
}
