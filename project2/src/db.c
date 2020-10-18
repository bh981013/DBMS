#include "db.h"
extern int fd;

int open_table(char *pathname){
	open_data(pathname);
	return fd;
}

int db_find(uint64_t key, char* ret_val)
{
	int result = index_find(key, ret_val);
	if(result >= 0) return 0;
	else return -1;
}

int db_insert(uint64_t key, char* value)
{
	int result = insert(key, value);
	return result;
}

int db_delete(uint64_t key)
{
	int result = delete(key);
	return result;
}
