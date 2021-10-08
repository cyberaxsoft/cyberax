#ifndef SHTRIH_FUNC_H_
#define SHTRIH_FUNC_H_

guint64 get_date_time(void);

guint8 calc_crc(guint8* buffer, guint8 length);

guint8 prepare_enq_frame(guint8* buffer);
guint8 prepare_nak_frame(guint8* buffer);
guint8 prepare_ack_frame(guint8* buffer);
guint8 prepare_get_state_frame(guint8* buffer);

const gchar* command_to_str(ShtrihCommand command);
const gchar* error_to_str(ShtrihError error);
const gchar* shtrih_mode_to_str(ShtrihMode mode);
const gchar* shtrih_doc_opened_status_to_str(ShtrihDocOpenedStatus status);
const gchar* shtrih_fiscal_a4_status_to_str(ShtrihFiscalA4Status status);
const gchar* shtrih_print_a4_status_to_str(ShtrihPrintA4Status status);
const gchar* shtrih_submode_to_str(ShtrihSubMode submode);

#endif /* SHTRIH_FUNC_H_ */
