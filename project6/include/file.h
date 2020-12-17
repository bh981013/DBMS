#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>


#define leaf_order 32
#define internal_order 239
#define page_size 4096
#define header_page_num 0


typedef uint64_t pagenum_t;



typedef struct record{
        uint64_t key;
        char value[120];
 }record;

typedef struct key_pnum{
	uint64_t key;
	pagenum_t pnum;
}key_pnum;

typedef struct header{
		 pagenum_t free_pnum;
		 pagenum_t root_pnum;
		 uint64_t num_of_p;
		 char reserved[4072];
	 	}header;

typedef struct free_page{
      	uint64_t next_pnum;
		char reserved[4088];
 		}free_;

typedef struct leaf{
		pagenum_t parent_pnum;
		uint32_t is_leaf;
		uint32_t num_of_k;
		char reserved1[8];
		uint64_t pageLSN;
		char reserved[88];
		pagenum_t r_pnum;
		record records[leaf_order - 1];	
		}leaf;

typedef struct internal{
		pagenum_t parent_pnum;
		uint32_t is_leaf;
		uint32_t num_of_k;
		char reserved1[8];
		int64_t pageLSN;
		char reserved[88];
		pagenum_t l_pnum;
		key_pnum keys[internal_order - 1];		
		}internal;

typedef struct page_t{
	union{
		header header_page;
		free_ free_page;
		leaf leaf_page;
		internal internal_page;
	};	
}page_t;

typedef struct buf_str{
	page_t frame;
	int table_id;
	pagenum_t pagenum;
	int is_dirty;
	pthread_mutex_t page_latch;
	int older_index; 
	int newer_index;
}buf_str;

typedef struct buf_info_str{
	int buf_size;
	int use_size; //buf에 새로할당될때 증가
	int LRU_old; 
	int LRU_new;
	pthread_mutex_t buf_latch;
}buf_info_str;

typedef struct table_str{
	int fd;
	char pathname[20];
	int is_open;
}table_str;

int log_fd;
FILE* log_out_fp;
void open_log_out(char* pathname);
void open_log_file(char* pathname);


extern table_str table_info[11];
extern buf_str* buf_arr;

int open_file(char* pathname, int* is_made);
page_t* make_free_page(int n);
pagenum_t file_alloc_page(int table_id);
void file_free_page(int table_id, pagenum_t pagenum);
void file_read_page(int table_id, pagenum_t pagenum, page_t* dest);
void file_write_page(int table_id, pagenum_t pagenum, const page_t* src);

