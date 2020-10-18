#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>


#define leaf_order 32
#define internal_order 249
#define page_size 4096
#define header_page_num 0

int fd;

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
		char reserved[104];
		pagenum_t r_pnum;
		record records[leaf_order - 1];	
		}leaf;

typedef struct internal{
		pagenum_t parent_pnum;
		uint32_t is_leaf;
		uint32_t num_of_k;
		char reserved[104];
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

int open_file(char* pathname, page_t* hpage);
page_t* make_free_page(int n);
pagenum_t file_alloc_page();
void file_free_page();
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_write_page(pagenum_t pagenum, const page_t* src);

