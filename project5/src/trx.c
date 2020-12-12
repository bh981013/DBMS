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
   // printf("lock_tsize:%ld", sizeof(lock_t));
    //printf("begin실행\n");
    pthread_mutex_lock(&trx_table_latch);
    trx_t* trx = (trx_t*)malloc(sizeof(trx_t));
    global_trx_id++;
    trx->trx_id = global_trx_id;
    trx->lock = NULL;
    trx->old_val = NULL;
    pthread_mutex_init(&(trx->trx_latch), 0);
    //printf("trx id: %d\n", trx->trx_id);
    HASH_ADD_INT(trx_table, trx_id, trx);
    pthread_mutex_unlock(&trx_table_latch);
    return trx->trx_id;
}

int trx_commit(int trx_id){
    pthread_mutex_lock(&lock_table_latch);
    pthread_mutex_lock(&trx_table_latch);

    
    trx_t* trx;
    HASH_FIND_INT(trx_table, &trx_id, trx);
    if(trx == NULL) return 0;
    lock_t* lock = trx->lock;
    //printf("진행\n");
    
    //printf("trx_id: %d commmit 실행-", trx_id);
    while(lock != NULL){
        lock_release(lock);
        lock = lock->trx_next;
    }
   // printf("-trx_id: %d commmit 끝\n", trx_id);

    
    //해당 trx삭제
    HASH_DEL(trx_table, trx);
   
    free(trx);
    pthread_mutex_unlock(&trx_table_latch);
    pthread_mutex_unlock(&lock_table_latch);
    return trx_id;
}
