#include "db.h"

int init_db(int buf_num , int flag, int log_num, char* log_path, char* logmsg_path){
	init_index(buf_num);
	init_log_buf();

	open_log_file(log_path);
	open_log_out(logmsg_path);

	recover(flag, log_num);

	flush_log();
	shutdown_db();
	return 0;
}


int open_table(char *pathname){
	int table_id = open_data(pathname);
	//printf("table id: %d open\n", table_id);
	return table_id;
}

int db_update(int table_id, int64_t key, char* values, int trx_id){
	char ret_val[120];
	int success;
	//int exist = find(table_id, key, ret_val);
	//if(exist == -1) return -1; //not found;
	
		success = update(table_id, key, values, trx_id);
	
	return success;
}


int db_find(int table_id, uint64_t key, char* ret_val, int trx_id)
{
	if(table_info[table_id].is_open == 0){
		//printf("closed table id\n");
		return -1;
	}
	int result = trx_find(table_id, key, ret_val, trx_id);
	return result;
}



int db_insert(int table_id, uint64_t key, char* value)
{
	if(table_info[table_id].is_open == 0){
		printf("closed table id\n");
		return -1;
	}
	int result = insert(table_id, key, value);
	return result;
}

int db_delete(int table_id, uint64_t key)
{
	if(table_info[table_id].is_open == 0){
		printf("closed table id\n");
		return -1;
	}
	int result = delete(table_id, key);
	return result;
}

int close_table(int table_id){
	if(buf_arr == NULL){
	printf("버퍼없당\n");
	return -1;
	}
	
	else flush_file(table_id);
	return 0;
}

int shutdown_db(){
	if(buf_arr == NULL){
	printf("no buffer existing\n");
	return -1;
	}
	for(int i = 1; i <= 10; i++){
		//printf("현재 테이블 size:%d\n", table_info[0].size);
		flush_file(i);
	}
	free(buf_arr);
	buf_arr = NULL;
	return 0;
}
