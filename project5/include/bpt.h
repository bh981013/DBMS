#ifndef __BPT_H__
#define __BPT_H__
#pragma once
// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "lock_table.h"
//#include "buffer.h"

table_str table_info[11]; //index가 pagenum 될수있게함.
	////0번째 인덱스에서는 저장된 개수 저장

#ifdef WINDOWS
#define bool char
#define false 0
#define true 1
#endif

#define LICENSE_FILE "LICENSE.txt"
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625
void init_index(int buf_num);
int find_table(char* pathname);

int open_data(char* pathname);

page_t get_header_page(int pid);
page_t get_root_page(int pid);

/*int index_find(uint64_t key, char* ret_val);
int index_find_2(uint64_t key);*/






// Output and utility.



pagenum_t find_leaf(int pid, uint64_t key);

/*trx 포함 함수들*/
pagenum_t trx_find_leaf(int pid, uint64_t key);
//void my_abort(int trx_id);
int update(int tid, int64_t key, char* value, int trx_id);
val_t* next_val(int trx_id);
int trx_find(int tid, uint64_t key, char* ret_val, int trx_id);


int find(int pid, uint64_t key, char* ret_val);

int cut( int length );

page_t make_internal_page(void);

page_t make_leaf_page(void);


int get_left_index(page_t ppage, pagenum_t l_pnum);

int insert_into_leaf(int pid, pagenum_t pagenum, uint64_t key, char* value);

void insert_into_leaf_after_splitting(int pid, pagenum_t pagenum, uint64_t key, char* value);

void insert_into_node(int pid, pagenum_t p_pagenum, int right_index, uint64_t key, pagenum_t r_pnum);

void insert_into_node_after_splitting(int pid, pagenum_t p_pnum, int right_index, uint64_t key, pagenum_t r_pnum);

int insert_into_parent(int pid, pagenum_t lnum, pagenum_t rnum, uint64_t new_key);

void insert_into_new_root(int pid, pagenum_t l_pagenum, pagenum_t r_pagenum, uint64_t key);

void start_new_tree(int pid, uint64_t key, char* value);

int insert(int pid, uint64_t key, char* value );

// Deletion.

void remove_entry_from_page(int pid, pagenum_t pagenum, int key);
int adjust_root(int pid);
int coalesce_pages(int pid, int k_index, int neighbor_index, pagenum_t c_pnum);
int redistribute_pages(int pid, pagenum_t p_pnum, int c_index);
int get_neighbor_index(int pid, pagenum_t pagenum);
int delete_entry(int pid, pagenum_t pagenum, uint64_t key);
int delete(int pid, uint64_t key);


#endif /* __BPT_H__*/
