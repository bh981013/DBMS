#ifndef __BPT_H__
#define __BPT_H__

// Uncomment the line below if you are compiling on Windows.
// #define WINDOWS
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "file.h"


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



void open_data(char* pathname);

page_t get_next_page(int i, page_t ipage);
page_t get_header_page();
page_t get_root_page();

int index_find(uint64_t key, char* ret_val);
int index_find_2(uint64_t key);






// Output and utility.



page_t find_leaf(uint64_t key, pagenum_t * ret_num);


int cut( int length );

page_t make_internal_page(void);

page_t make_leaf_page(void);


int get_left_index(page_t ppage, pagenum_t l_pnum);

int insert_into_leaf(pagenum_t pagenum, page_t l_page, uint64_t key, char* value);

void insert_into_leaf_after_splitting(pagenum_t pagenum, page_t lpage, uint64_t key, char* value);

void insert_into_node(pagenum_t p_pagenum, page_t ppage, int left_index, uint64_t key, pagenum_t r_pagenum);

void insert_into_node_after_splitting(pagenum_t p_pnum, page_t ppage, int left_index, uint64_t key, pagenum_t r_pnum);

int insert_into_parent(pagenum_t lnum, pagenum_t rnum, page_t lpage, uint64_t new_key, page_t rpage);

void insert_into_new_root(pagenum_t l_pagenum, pagenum_t r_pagenum, page_t lpage, uint64_t key, page_t rpage);

void start_new_tree(uint64_t key, char* value);

int insert(uint64_t key, char* value );

// Deletion.

void remove_entry_from_page(int key);
int adjust_root();
int redistribute_pages(pagenum_t p_pnum, int index, int neighbor_index);
int delete_entry(uint64_t key);
int delete(uint64_t key);


#endif /* __BPT_H__*/
