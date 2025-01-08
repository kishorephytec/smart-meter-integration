
#ifndef DLMS_H_
#define DLMS_H_

#include <libxml/parser.h>
#include <libxml/tree.h>

char* call_decode_message(const char *hex_message, int len);

#endif /* DLMS_H_ */