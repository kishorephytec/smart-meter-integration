/*
 * database.h
 *
 *  Created on: 16-Dec-2024
 *      Author: Srikanth Jogi
 */


#include <sqlite3.h>
#ifndef DATABASE_H
#define DATABASE_H

// Function declarations
int initialize_database(); // Initializes the database and creates the table
void store_data(const char *type, const char *value); // General-purpose data storage function
						      
void store_network_sync_info(const char *node_id, const char *parent_id, const char *serial_number, const char *connectivity_status);
int initialize_network_syncDB();

char* fetch_modem_number();
char* fetch_node_id(unsigned char* system, unsigned short system_len);
char* fetch_meter_id(const char* global_system, unsigned short system_len);

void clear_network_db();

void store_serial_number(const char *serial_number);
void store_network_size(int network_size);
void store_nc_database_size(int nc_db_size);
void store_network_id(const char *network_id);
void store_node_id(const char *node_id);

void trim_string(char *str);
void insert_node_info(sqlite3 *db, const char *node_id, const char *parent_id, const char *serial_number, const char *meter_serial_number, const char *connectivity_status);

#endif // DATABASE_H

