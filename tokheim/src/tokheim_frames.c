#include <glib.h>
#include <stdio.h>

#include "logger.h"
#include "driver.h"
#include "tokheim.h"
#include "config.h"
#include "tokheim_func.h"

#include "driver_state.h"

guint8 prepare_get_fuelling_point_id_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_FuellingPointId_req);

	return result;
}

guint8 prepare_get_aux_fuelling_point_id_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_AuxFuellingPointId_req);

	return result;
}

guint8 prepare_get_fuelling_point_display_data_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_FuellingPointDisplayData_req);

	return result;
}

guint8 prepare_get_fuelling_point_status_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_FuellingPointStatus_req);

	return result;
}

guint8 prepare_suspend_fuelling_point_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_SuspendFuellingPoint);

	return result;
}

guint8 prepare_resume_fuelling_point_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_ResumeFuellingPoint);

	return result;
}

guint8 prepare_authorize_fuelling_point_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume)
{
	guint8 result = 0;

	guint8 price_dp = 0;
	guint8 volume_dp = 0;
	guint8 amount_dp = 0;


	safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_AuthorizeFuellingPoint);
	result+=add_byte_to_buffer(&buffer[result], SLOW_FLOW_OFFSET);

	result+=add_value_to_buffer(&buffer[result], price, PRICE_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], mult_price_litres(price, volume, volume_dp), AMOUNT_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], volume, VOLUME_FIELD_SIZE);

	return result;
}

guint8 prepare_authorize_fuelling_point2_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume)
{
	guint8 result = 0;

	guint8 price_dp = 0;
	guint8 volume_dp = 0;
	guint8 amount_dp = 0;


	safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_AuthorizeFuellingPoint2);
	result+=add_byte_to_buffer(&buffer[result], SLOW_FLOW_OFFSET);

	result+=add_value_to_buffer(&buffer[result], price, PRICE_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], mult_price_litres(price, volume, volume_dp), AMOUNT_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], volume, VOLUME_FIELD_SIZE);

	return result;
}

guint8 prepare_send_data_for_fuelling_point_frame(guint8* buffer, guint8 disp_addr, guint32 price, guint32 volume)
{
	guint8 result = 0;

	guint8 price_dp = 0;
	guint8 volume_dp = 0;
	guint8 amount_dp = 0;


	safe_get_decimal_point_positions(&price_dp, &volume_dp, &amount_dp);

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_SendDataForFuellingPoint);

	result+=add_value_to_buffer(&buffer[result], price, PRICE_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], mult_price_litres(price, volume, volume_dp), AMOUNT_FIELD_SIZE);
	result+=add_value_to_buffer(&buffer[result], volume, VOLUME_FIELD_SIZE);

	return result;
}

guint8 prepare_reset_fuelling_point_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_ResetFuellingPoint);

	return result;
}

guint8 prepare_fuelling_point_totals_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_FuellingPointTotals_req);

//	if (safe_get_nozzle_count(disp_index) > 1)
//	{
		result+=add_byte_to_buffer(&buffer[result], 0x00);
//	}

	return result;
}

guint8 prepare_set_fuelling_point_display_control_frame(guint8* buffer, guint8 disp_addr, gboolean light_on)
{
	guint8 result = 0;

	guint8 control_byte = 0;  // money 2

	control_byte |= 1 << 4; // volume 2

	if (light_on)
	{
		control_byte |= 1 << 2;
	}

	control_byte |= 1; // price 2

	result+=add_byte_to_buffer(&buffer[result], FP_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tc_SetFuellingPointDisplayControl);
	result+=add_byte_to_buffer(&buffer[result], control_byte);

	return result;
}

guint8 prepare_request_activated_hose_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], EXT_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tac_ActivatedHose_req);

	return result;
}

guint8 prepare_request_deactivated_hose_frame(guint8* buffer, guint8 disp_addr)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], EXT_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tac_DeactivatedHose_req);

	return result;
}

guint8 prepare_send_cash_prices_frame(guint8* buffer, guint8 disp_addr, guint32 price1, guint32 price2, guint32 price3)
{
	guint8 result = 0;

	result+=add_byte_to_buffer(&buffer[result], EXT2_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tac_SendCashPrices_req);
	result+=add_reverse_value_to_buffer(&buffer[result], price1, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price2, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price3, PRICE_FIELD_SIZE);

	return result;
}

guint8 prepare_send_aux_cash_prices_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr)
{
	guint8 result = 0;

	guint32 price1 = safe_get_nozzle_price(disp_index, 0);
	guint32 price2 = safe_get_nozzle_price(disp_index, 1);
	guint32 price3 = safe_get_nozzle_price(disp_index, 2);
	guint32 price4 = safe_get_nozzle_price(disp_index, 3);

	result+=add_byte_to_buffer(&buffer[result], EXT_MASK | (disp_addr - 1));
	result+=add_byte_to_buffer(&buffer[result], tac_SendCashPrices_req);
	result+=add_reverse_value_to_buffer(&buffer[result], price1, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price1, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price2, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price2, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price3, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price3, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price4, PRICE_FIELD_SIZE);
	result+=add_reverse_value_to_buffer(&buffer[result], price4, PRICE_FIELD_SIZE);

	return result;
}

