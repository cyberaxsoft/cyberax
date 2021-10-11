#ifndef TLS_FRAMES_H_
#define TLS_FRAMES_H_

guint8 prepare_select_channel_frame(guint8* buffer, guint8 active_channel);
guint8 prepare_channel_configuration_frame(guint8* buffer);
guint8 prepare_request_parameters_frame(guint8* buffer);

#endif /* TLS_FRAMES_H_ */
