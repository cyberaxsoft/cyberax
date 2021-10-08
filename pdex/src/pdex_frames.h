#ifndef PDEX_FRAMES_H_
#define PDEX_FRAMES_H_

guint8 pdex_prepare_ctrl(guint8* buffer, guint8 channel, guint8 symbol);
guint8 pdex_prepare_command(guint8* buffer, guint8 channel, guint8 command, guint8* data, guint8 data_size);

#endif /* PDEX_FRAMES_H_ */
