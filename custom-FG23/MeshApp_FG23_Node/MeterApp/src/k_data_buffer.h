#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include <string.h>
#include <stdint.h>

#define APPND -1

typedef struct
{
    uint8_t* data_buffer;
    uint16_t data_ptr;
    uint16_t data_size;
    uint16_t data_capacity;
}data_struct_t;

typedef enum
{
    STR_TYPE_NULL,
    STR_TYPE_OCTAT = 9,
    STR_TYPE_VISIBLE = 10
}string_type_e;

typedef union 
{
    uint8_t uint8;
    uint32_t uint32;
    uint64_t uint64;
    int8_t int8;
    int16_t uint16;
    int32_t int32;
    float float32;
}type_conv_u;

void init_data_struct(data_struct_t* buff, uint8_t* data, uint16_t length);
void clear_data_buffer(data_struct_t* buff);
void copy_data_buffer(data_struct_t* buff, const uint8_t* data, uint16_t data_len, uint8_t AppendData);
uint8_t get_8(data_struct_t* buff);
uint16_t get_16(data_struct_t* buff);
uint32_t get_32(data_struct_t* buff);
uint64_t get_64(data_struct_t* buff);
void set_8(data_struct_t* buff, uint8_t value, int16_t index);
void set_16(data_struct_t* buff, uint16_t value, int16_t index);
void set_32(data_struct_t* buff, uint32_t value, int16_t index);
void set_uint64(data_struct_t* buff, uint64_t value, int16_t index);
void set_string(data_struct_t* buff, string_type_e string_type, uint8_t* data, uint16_t data_len);
#endif//DATA_BUFFER_H_
