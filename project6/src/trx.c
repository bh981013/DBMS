#include "trx.h"
#include "lock_table.h"
int global_trx_id = 0;
pthread_mutex_t trx_table_latch = PTHREAD_MUTEX_INITIALIZER;
trx_t* trx_table = NULL;

pthread_mutex_t* get_trx_table_latch(){
   pthread_mutex_t* trx_table_latch_pointer = &trx_table_latch;
    return trx_table_latch_pointer;
}

trx_t* get_trx_table(){
    return trx_table;
}

int trx_begin(){
    //printf("lock_tsize:%ld", sizeof(lock_t));
   //printf("trx: %d begin 시도\n", global_trx_id + 1);
    pthread_mutex_lock(&trx_table_latch);
     
    trx_t* trx = (trx_t*)malloc(sizeof(trx_t));
    global_trx_id++;
    trx->trx_id = global_trx_id;
    trx->lock = NULL;
    trx->old_val = NULL;
    printf("trx_id: %d begin실행\n", trx->trx_id);
    pthread_mutex_init(&(trx->trx_latch), 0);
    //printf("trx id: %d\n", trx->trx_id);
    make_begin_log(trx->trx_id);

    HASH_ADD_INT(trx_table, trx_id, trx);
    pthread_mutex_unlock(&trx_table_latch);
    printf("trx: %d begin 종료\n", trx->trx_id);
    return trx->trx_id;
}

int trx_commit(int trx_id){
    //
    
   //printf("trx_id: %d의 commit 시도\n", trx_id);
    pthread_mutex_lock(&lock_table_latch);
   // printf("trx_id: %d의 commit 다시한번시도\n", trx_id);
    pthread_mutex_lock(&trx_table_latch);

    
    trx_t* trx;
    HASH_FIND_INT(trx_table, &trx_id, trx);
    if(trx == NULL) {
        //printf("이미삭제됨\n");
        pthread_mutex_unlock(&trx_table_latch);
        pthread_mutex_unlock(&lock_table_latch);
        return 0;
    }
    lock_t* lock = trx->lock;
    //printf("진행\n");
    
    printf("trx_id: %d commmit 실행-", trx_id);
    while(lock != NULL){
        lock_release(lock);
        lock = lock->trx_next;
    }
    printf("-trx_id: %d commmit 끝\n", trx_id);
    for(int j = 0; j<500; j++){
        arr[j][trx_id%500] = 0;
		arr[trx_id%500][j] = 0;
	}

    make_commit_log(trx_id);
    flush_log();

    HASH_DEL(trx_table, trx);
    free(trx);
    pthread_mutex_unlock(&trx_table_latch);
    pthread_mutex_unlock(&lock_table_latch);
    return trx_id;
}

void trx_abort(int trx_id){
	//pthread_mutex_lock(&lock_table_latch);
	//printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
	
	//printf("전체 중%d번째\n trx:%d abort()\n", i, trx_id);
	trx_t* trx;
	HASH_FIND_INT(get_trx_table(), &trx_id, trx);
	lock_t* lock = trx->lock;
	val_t* val_str;
	node_t* node;
	page_t page;
	while(lock != NULL){
		node = lock->sent;
		//print_node(node);
		if(lock->next == NULL && lock->lock_mode == 1){
			val_str = trx->old_val;
			//printf("val_str_p: %p\n", val_str);
			//printf("tid: %d, pnum: %ld\n", val_str->tid, val_str->pnum);
            int index = buf_read_frame(val_str->tid, val_str->pnum, &page);

            make_update_log(trx_id, val_str->tid, val_str->pnum, val_str->index,
             val_str->val, page.leaf_page.records[val_str->index].value);
			
			
			strcpy(page.leaf_page.records[val_str->index].value, val_str->val);
			buf_write_frame(index, val_str->tid, val_str->pnum, &page);
			trx->old_val = val_str->next;


		}
		//printf("abort후 노드의 모습:\n");
		//print_node(node);
		pthread_mutex_lock(&lock_table_latch);
		pthread_mutex_lock(get_trx_table_latch());
		lock_release(lock);
		pthread_mutex_unlock(get_trx_table_latch());
		 pthread_mutex_unlock(&lock_table_latch);
		lock = lock->trx_next;
	}
	trx->lock = NULL;

    make_rollback_log(trx_id);

    flush_log();


}