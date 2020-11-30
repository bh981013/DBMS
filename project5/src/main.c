/*#include "file.h"
#include "db.h"
#include "bpt.h"

#define thread_num 40

void print_header(int tid){
	page_t header_page;
	file_read_page(tid, 0, &header_page);
	printf("헤더정보: root_pnum:%ld free_pnum:%ld\n\n", header_page.header_page.root_pnum, header_page.header_page.free_pnum);
}

void print_root(int tid){
	int i;
	page_t hpage;
	file_read_page(tid, 0, &hpage);
	page_t rpage;
	file_read_page(tid, hpage.header_page.root_pnum, &rpage);
	
	printf("루트정보: numofk:%d\n ",rpage.internal_page.num_of_k );

	if(rpage.internal_page.is_leaf == 0)
	{
		for(i = 0; i<rpage.leaf_page.num_of_k; i++)
		{
		printf("key%d: %ld ",i, rpage.internal_page.keys[i].key );
		}
	}

	else{
		for(i = 0; i<rpage.leaf_page.num_of_k; i++)
		{
		printf("key%d: %ld ",i, rpage.leaf_page.records[i].key );
		}
	}	
	printf("\n");
}

void print_page(int pid, pagenum_t pagenum){
	page_t page;
	int i;
	file_read_page(pid, pagenum, &page);
	printf("\n%ld번째 페이지 정보: num_of_k:%d  is_leaf:%d, parent_pnum:%ld\n ", pagenum, page.internal_page.num_of_k, page.internal_page.is_leaf, page.internal_page.parent_pnum);
	if(page.internal_page.is_leaf == 0)
	{
		for(i = 0; i<page.leaf_page.num_of_k; i++)
		{
		printf("key%d: %ld  ",i, page.internal_page.keys[i].key);
		}
			
	}

	else{
		for(i = 0; i<page.leaf_page.num_of_k; i++)
		{
		printf("key%d: %ld",i, page.leaf_page.records[i].key);
		}
		printf("\n");
		for(i = 0; i<page.leaf_page.num_of_k; i++)
		{
		printf("value%d: %s ",i, page.leaf_page.records[i].value );
		}

	}
	printf("\n");
}
typedef struct arg{
	int tid;
	uint64_t key;
	char* value;
}arg_t;

void* thread_insert(){
	for(int i = 1; i< 15; i++){
	db_insert(1, i, "a");
	//db_insert(2, i, "b");
	}
	close_table(1);
	//close_table(2);
	print_header(1);
	print_root(1);
	for(int i = 1000; i>=990; i--){
		print_page(1, i);
	}

	print_header(2);
	print_root(2);
	for(int i = 1000; i>=990; i--){
		print_page(2, i);
	}
}
arg_t get_arg(int tid, uint64_t key, char* value){
	arg_t arg;
	arg.tid = tid;
	arg.key = key;
	arg.value = value;
	return arg;
}

void* test(void* arg){
	int id = db_begin();
	open_table("1");
	arg_t* a = (arg_t*) arg;
	uint64_t i = a->key;
	char val[120];
	if(db_find(1, i, val, id) != 0) return NULL;
	printf("\n\nkey: %ld의 %s를 찾았다!!\n\n", a->key, val);
	if((db_update(1, i, "BABY!!!!", id) != 0)) return NULL;
	close_table(1);
	db_commit(id);
}

void* test2(void* arg){
	int id = db_begin();
	open_table("1");
	arg_t* a = (arg_t*) arg;
	uint64_t i = a->key;
	char val[120];
	if(db_find(1, i, val, id) != 0) return NULL;
	//printf("\n\nkey: %ld의 %s를 찾았다!!\n\n", a->key, val);
	if((db_update(1, i, "BABY!!!!", id) != 0)) return NULL;
	close_table(1);
	db_commit(id);
	printf("test2끝\n");
}




int main()
{
	init_db(10);
	init_lock_table();
	open_table("1");
	//open_table("2");
	pthread_t id;
	int ret;
	pthread_create(&id, NULL, thread_insert, NULL);
	pthread_join(id, (void**)&ret);
	char input;

	printf("\n\n\n\n\n\n\n\n\n\n");
	pthread_t threads[thread_num];
	pthread_t threads2[thread_num];
	for(uint64_t i = 0; i < thread_num; i++){
		arg_t* arg = (arg_t*)malloc(sizeof(arg_t));
		arg->key = 1;
		printf("key: 1");
		pthread_create(&threads[i], NULL, test, (void*)arg);
	}

	for(uint64_t i = 0; i < thread_num; i++){
		arg_t* arg = (arg_t*)malloc(sizeof(arg_t));
		arg->key =  2;
		printf("key: %ld", arg->key);
		pthread_create(&threads2[i], NULL, test2, (void*)arg);
	}


	for(int j = 0; j < thread_num; j++){
		pthread_join(threads[j], NULL);
		pthread_join(threads2[j], NULL);
	}

	printf("끝!\n");
	for(int i = 1000; i>=990; i--){
		print_page(1, i);
	}

	scanf("%c", &input);
	if(input == 'p'){
		print_buf();
	}
	

}


	int i = 1;
	int j = 0;

	for(j = 1; j <= 9; j++){
	insert(j,"abc");
	}

	
	while(i){
	scanf("%d", &i);
	if(i == 0) break;
	insert(i,"abc");x

 	print_page((pagenum_t)1000);
	print_page((pagenum_t)999);
	print_page((pagenum_t)998);
	print_page((pagenum_t)997);
	print_page((pagenum_t)996);
	print_page((pagenum_t)995);
	print_page((pagenum_t)994);
print_page((pagenum_t)993);
	}
	i = 1;
	printf("이제뭘지울래?");
	while(i){
	scanf("%d", &i);
	delete(i);
	print_page((pagenum_t)1000);
	print_page((pagenum_t)999);
	print_page((pagenum_t)998);
	print_page((pagenum_t)997);
	print_page((pagenum_t)996);
	print_page((pagenum_t)995);
	print_page((pagenum_t)994);
print_page((pagenum_t)993);
	}
	print_page((pagenum_t)1000);
	print_page((pagenum_t)999);
	print_page((pagenum_t)998);
	print_page((pagenum_t)997);
	print_page((pagenum_t)996);
	print_page((pagenum_t)995);
	print_page((pagenum_t)994);
print_page((pagenum_t)993);
	print_header();
	print_root();*/


int main(){
	return 0;
}










