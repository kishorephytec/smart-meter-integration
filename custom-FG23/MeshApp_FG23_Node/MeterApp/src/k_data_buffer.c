#include "k_data_buffer.h"


void init_data_struct(data_struct_t* buff, uint8_t* data, uint16_t length)
{
    buff->data_buffer = data;
    buff->data_ptr = 0;
    buff->data_size = 0;
    buff->data_capacity = length;
}

void clear_data_buffer(data_struct_t* buff)
{
    buff->data_buffer = 0;
    buff->data_ptr = 0;
    buff->data_size = 0;
    buff->data_capacity = 0;
}
void copy_data_buffer(data_struct_t* buff, const uint8_t* data, uint16_t data_len, uint8_t AppendData)
{
    if (buff->data_capacity >= (buff->data_size + data_len))
    {
        if (!AppendData)
        {
            buff->data_size = 0;
        }
        memcpy(&buff->data_buffer[buff->data_size], data, data_len);
        buff->data_size += data_len;
    }
}

uint8_t get_8(data_struct_t* buff)
{
    uint8_t ret = 0;
    if (buff->data_capacity >= (buff->data_size + 1))
    {
        ret = buff->data_buffer[buff->data_ptr++];
    }
    return ret;
}
uint16_t get_16(data_struct_t* buff)
{
    uint16_t ret = 0;
    if (buff->data_capacity >= (buff->data_size + 2))
    {
        ret = ((uint16_t)(((buff->data_buffer[buff->data_ptr] & 0xFF)) << 8) |
            ((uint16_t)(buff->data_buffer[buff->data_ptr + 1] & 0xFF)));
        buff->data_ptr += 2;
    }
    return ret;
}
uint32_t get_32(data_struct_t* buff)
{
    uint32_t ret = 0;
    if (buff->data_capacity >= (buff->data_size + 4))
    {
        ret = ((uint32_t)((buff->data_buffer[buff->data_ptr] & 0xFF)) << 24 |
            ((uint32_t)(buff->data_buffer[buff->data_ptr + 1] & 0xFF)) << 16 |
            ((uint32_t)(buff->data_buffer[buff->data_ptr + 2] & 0xFF)) << 8 |
            ((uint32_t)(buff->data_buffer[buff->data_ptr + 3] & 0xFF)));
        buff->data_ptr += 4;
    }
    return ret;
}

uint64_t get_64(data_struct_t* buff)
{
    uint64_t ret = 0;
    if (buff->data_capacity >= (buff->data_size + 8))
    {
        ret = (((uint64_t)buff->data_buffer[buff->data_ptr] & 0xFF) << 56) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 1] & 0xFF) << 48) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 2] & 0xFF) << 40) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 3] & 0xFF) << 32) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 4] & 0xFF) << 24) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 5] & 0xFF) << 16) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 6] & 0xFF) << 8) |
            (((uint64_t)buff->data_buffer[buff->data_ptr + 7] & 0xFF));
        buff->data_ptr += 8;
    }
    return ret;
}

uint16_t get_strings_length(data_struct_t* buff)
{
    uint16_t length = get_8(buff);
    if (length < 0x80)
    {
        return length;
    }
    else
    {
        length = get_8(buff);
        length = length << 8;
        length |= get_8(buff);
    }
    return length;
}


void set_8(data_struct_t* buff, uint8_t value, int16_t index)
{
    if (index <= 0)
    {
        index = buff->data_size++;
    }
    buff->data_buffer[index] = value;
}
void set_16(data_struct_t* buff, uint16_t value, int16_t index)
{
    if (index <= 0)
    {
        index = buff->data_size;
        buff->data_size += 2;
    }
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 8);
    buff->data_buffer[index] = 0xFF & (uint8_t)(value);
}
void set_32(data_struct_t* buff, uint32_t value, int16_t index)
{
    if (index <= 0)
    {
        index = buff->data_size;
        buff->data_size += 4;
    }
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 24);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 16);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 8);
    buff->data_buffer[index] = 0xFF & (uint8_t)(value);
}

void set_uint64(data_struct_t* buff, uint64_t value, int16_t index)
{

    if (index <= 0)
    {
        index = buff->data_size;
        buff->data_size += 4;
    }
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 56);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 48);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 40);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 32);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 24);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 16);
    buff->data_buffer[index++] = 0xFF & (uint8_t)(value >> 8);
    buff->data_buffer[index] = 0xFF & (uint8_t)(value);
}

void set_string(data_struct_t* buff, string_type_e string_type, uint8_t* data, uint16_t data_len)
{
    if (string_type != STR_TYPE_NULL)
    {
        set_8(buff, string_type, APPND);
        if (data_len < 0x80)
        {
            set_8(buff, (uint8_t)data_len, APPND);
        }
        else
        {
            set_8(buff, 0x82, APPND);
            set_16(buff, data_len, APPND);
        }
    }
    copy_data_buffer(buff, data, data_len, 1);
}
