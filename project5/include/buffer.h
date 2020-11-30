#pragma once

#include <pthread.h>
#include <string.h>
#include "file.h"
//#include "bpt.h"

#define path_size 20
#define path_arr_size 10


buf_str* buf_arr;
buf_info_str buf_info;

void print_buf();
int flush_page(int table_id, pagenum_t pagenum);
int flush_file(int table_id);
buf_str* make_buf(int num);
int buf_find_frame(int table_id, pagenum_t pagenum);
int buf_read_frame(int table_id, pagenum_t pagenum, page_t* frame);
int buf_write_frame(int table_id, pagenum_t pagenum, page_t* frame);

void my_unlock(int index);

int flush_page(int table_id, pagenum_t pagenum);
pagenum_t buf_alloc_page(int table_id);
void buf_free_page(int pid, pagenum_t pagenum);
int evict_and_alloc();
