/*
 * dart_func.h
 *
 *  Created on: 8 февр. 2021 г.
 *      Author: mkv
 */

#ifndef DART_FUNC_H_
#define DART_FUNC_H_


const gchar* ds_to_str(DispencerState state);
void get_original_pump_status_description(guint8 status, guchar* buffer);
guint32 packed_bcd_to_bin(guint8* buff, guint8 size);
guint16 calc_crc(guint8* buffer, guint8 length);
guint64 get_date_time(void);
void get_error_description(guint8 error_code, guchar* buffer);

guint8 prepare_pool_frame(guint8* buffer, guint8 disp_addr);
guint8 prepare_status_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
guint8 prepare_command_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number);
guint8 prepare_suspend_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number, guint8 disp_index);
guint8 prepare_resume_pump_frame(guint8* buffer, guint8 disp_addr, DartCommandPump command, guint8 block_sequence_number, guint8 disp_index);
guint8 prepare_ack_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
guint8 prepare_nak_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number);
guint8 prepare_get_counter_frame(guint8* buffer, guint8 disp_addr, guint8 block_sequence_number, guint8 nozzle_index);

guint8 prepare_preset_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, DartCommand command, LogOptions log_options);
guint8 prepare_set_prices_frame(guint8* buffer, guint8 disp_index, guint8 disp_addr, guint8 block_sequence_number, LogOptions log_options);


#endif /* DART_FUNC_H_ */
