#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mosquitto.h>
#include <sys/utsname.h>
#include <time.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <system_ip.h>
#include <timestamp.h>

//Extern and Global variables
SystemIP *head = NULL;  // Head pointer for the linked list

//Addition of Meter ID w.r.t IP address
void insert_system(const char *ip_address, const char *id,char *timestamp) {
    
    SystemIP *current = head;
    // Check if the IP is already assigned to a system
    while (current != NULL) {
        if (memcmp(current->ip, ip_address, 16) == 0) {
            printf("\n%s [INFO][ERROR     ]: IP already assigned to system: %s\n", current->system);
            return;
        }
        current = current->next;
    }

    SystemIP *new_node = (SystemIP *)malloc(sizeof(SystemIP));
    if (new_node == NULL) {
        printf("\n%s [INFO][ERROR     ]: Memory allocation failed\n",timestamp);
        return;
    }

    //Assign IP with System name
    strcpy(new_node->system,id);
    memcpy(new_node->ip, ip_address,16);
    new_node->next = NULL;

    if (head == NULL) {
        head = new_node;
    } else {
        SystemIP *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
    printf("\n%s [INFO][System     ]: System %s with IP ",timestamp,new_node->system);
    for (int i = 0; i < 16; ++i) {
	                printf("%02X ", new_node->ip[i]);
	            }
    printf("added.\n");
}

void delete_system(const char *system_name, char *timestamp) {
    
    SystemIP *current = head;
    SystemIP *previous = NULL;

    if (current == NULL) {
        printf("\n%s [INFO][ERROR     ]:No systems available for deletion.\n",timestamp);
        return;
    }

    // Traverse the list to find the node to delete
    while (current != NULL && strcmp(current->system, system_name) != 0) {
        previous = current;
        current = current->next;
    }

    // If the system was not found
    if (current == NULL) {
        printf("\n%s [INFO][ERROR     ]:System %s not found.\n", timestamp,system_name);
        return;
    }

    // If the system is found at the head
    if (previous == NULL) {
        head = current->next;
    } else {
        previous->next = current->next;
    }

    free(current);
    printf("\n%s [INFO][ERROR     ]:System %s deleted.\n",timestamp, system_name);
}

//Informatuion of Meter ID
char *print_system_table(int *system_count) {
    // Initialize the system count to 0
    *system_count = 0;

    // Allocate a buffer for the formatted output (assume max length here for simplicity)
    size_t buffer_size = 1024;
    char *buffer = (char *)malloc(buffer_size);
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return NULL;
    }

    buffer[0] = '\0';

    // Traverse through the linked list and add each system name to the buffer
    SystemIP *current = head;
    while (current != NULL) {
        if (strlen(buffer) + strlen(current->system) + 2 > buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (buffer == NULL) {
                printf("Memory allocation failed during reallocation\n");
                return NULL;
            }
        }

        // Append the system name with a space to the buffer
        snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%s ", current->system);

        current = current->next;
        (*system_count)++;
    }

    return buffer;
}

/*******************************Environment info************************************************/
// Function to get IP address of an interface
static char *get_ip_address(const char *interface) {
    struct ifreq ifr;
    struct sockaddr_in *ipaddr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("Socket error");
        return NULL;
    }

    strcpy(ifr.ifr_name, interface);
    if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
        ipaddr = (struct sockaddr_in *)&ifr.ifr_addr;
        char *ip = strdup(inet_ntoa(ipaddr->sin_addr));
        close(sock);
        return ip;
    } else {
        perror("ioctl error");
        close(sock);
        return NULL;
    }
}

//Function to fetch SystemInfo
void fetch_system_info(char *buffer, size_t size) {
    struct utsname sys_info;

    if (uname(&sys_info) == 0) {
        snprintf(buffer + strlen(buffer), size - strlen(buffer),
                 "Build-name...... %s / %s\n"
                 "Version......... %s\n"
                 "Product-Id...... %s\n"
                 "Bootloader...... %s\n"
                 "Config/Status... %s\n"
                 "Radio........... %s\n",
                 sys_info.sysname, sys_info.nodename, sys_info.version,
                 sys_info.release, "b109", "629-1-21", "[V306_03355] 1.1");
    } else {
        perror("uname");
        strncat(buffer, "System info: Unknown\n", size - strlen(buffer));
    }
}

// Function to fetch network info
void fetch_network_info(char *buffer, size_t size, const char *interface) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return;
    }

    // MTU
    strcpy(ifr.ifr_name, interface);
    if (ioctl(sock, SIOCGIFMTU, &ifr) == 0) {
        snprintf(buffer + strlen(buffer), size - strlen(buffer), "MTU............. %d\n", ifr.ifr_mtu);
    } else {
        strncat(buffer, "MTU............. Unknown\n", size - strlen(buffer));
    }

    // IP Address
    char *ip_address = get_ip_address(interface);
    if (ip_address) {
        snprintf(buffer + strlen(buffer), size - strlen(buffer), "Unicast......... %s\n", ip_address);
        free(ip_address);
    } else {
        strncat(buffer, "Unicast......... Unknown\n", size - strlen(buffer));
    }


    close(sock);
}


