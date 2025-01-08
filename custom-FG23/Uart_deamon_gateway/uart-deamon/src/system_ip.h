#ifndef SYSTEM_IP_H
#define SYSTEM_IP_H

typedef struct SystemIP {
    char system[50];
    char ip[16];
    struct SystemIP *next;  // Pointer to the next system in the list
} SystemIP;

extern SystemIP *head;

typedef struct {
    unsigned char src_address[16];
    unsigned char dst_address[16];
    unsigned char counter[2];
    unsigned char type;
} IpPacket;

extern IpPacket packet;
void insert_system(const char *ip_address, const char *id,char *timestamp);
void delete_system(const char *system_name,char *timestamp);
char *print_system_table(int *system_count);
void fetch_system_info(char *buffer, size_t size);
void fetch_network_info(char *buffer, size_t size, const char *interface);
#endif // SYSTEM_IP_H


