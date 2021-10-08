
#ifndef XML_H_
#define XML_H_

gboolean read_settings(const gchar* filename, ServConfig* configuration);
gboolean write_settings(const gchar* filename, ServConfig* configuration);

#endif /* XML_H_ */
