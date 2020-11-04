#pragma once
#include <fcntl.h>
#include <unistd.h>
#include "bpt.h"
int init_db(int buf_num);
int open_table(char *pathname);
int db_find(int table_id, uint64_t key, char* ret_val);
int db_insert(int table_id, uint64_t key, char* value);
int db_delete(int table_id, uint64_t key);
int close_table(int table_id);
int shutdown_db();
