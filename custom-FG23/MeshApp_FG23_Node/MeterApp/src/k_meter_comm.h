
#include <stdint.h>
#define MAX_UART_DATA_BUFF_SIZE 3000
#define MIN_DLMS_BYTES 8

#define VERSION_NUMBER          0x0001
#define SOURCE_ADDRESS          0x0001
#define CONFIG_ADDRESS          0x0002

#define LOWEST_LEVEL_ASSN       0x0010
#define LOW_LEVEL_ASSN          0x0020
#define HIGH_LEVEL_ASSN         0x0030
#define PUSH_ASSOCIATION        0x0040
#define FW_UPDATE_ASSN          0x0050
typedef struct
{
    uint8_t data[MAX_UART_DATA_BUFF_SIZE];
    uint16_t data_len;
    uint16_t data_ptr;
    uint16_t read_ptr;
    uint16_t timeout;
}ring_buff_t;

typedef enum 
{
    UART_DLMS_OK,
    UART_DLMS_NO_DATA,
    UART_DLMS_INVALID_DATA, 
    UART_DLMS_INCOMPLETE_DATA,
    UART_DLMS_MEMEORY_OVERFLOW,
}uart_dlms_data_e;

typedef struct uart_dlms_data_struct
{
    //struct uart_dlms_data_struct* next; //later store in queue
    uint8_t data[1500];
    uint16_t data_len;
    uint16_t association;
}uart_dlms_data_t;

extern uart_dlms_data_t ud_dlms;

typedef union
{
    uint8_t u8_val[4];
    uint16_t u16_val[2];
    uint32_t u32_val;
}un_data_t;

typedef void(*uart_dlms_callback_t)(const uart_dlms_data_t* ud);
void register_dlms_callback(uart_dlms_callback_t cb);
void get_dlms_data(uint8_t* error_code);
void k_init_meter_comm(void);
void k_init_info_plus_meter_comm(void);