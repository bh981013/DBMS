#pragma once
#include <fcntl.h>
#include <unistd.h>
#include "bpt.h"

int open_table(char *pathname);
int db_find(uint64_t key, char* ret_val);
int db_insert(uint64_t key, char* value);
int db_delete(uint64_t key);
