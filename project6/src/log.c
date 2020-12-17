#include "log.h"
#include "buffer.h"


log_record_t* winners = NULL;
log_record_t* losers = NULL;
log_record_t* all = NULL;



void init_log_buf(){
    pthread_mutex_init(&log_buf_latch, NULL);
    current_log_index = 0;
    flushed_index = 0;
    return;
}

int recover(int flag, int log_num){
   
    analysis();
    
    if(flag == 1) redo(log_num);
    else redo(0);

    if(flag == 2) undo(log_num);
    else undo(0);

}



void analysis(){
    
    fprintf(log_out_fp, "[ANALYSIS]: Analysis pass start\n");
   
    int index = 0;
    log_record_t log_record;
    log_record_t* put_log_record = malloc(sizeof(log_record_t));
    while(log_read_record(log_fd, index, &log_record)){
        if (log_record.type == BEGIN){
             //add done_trx(trx_id, is_winner = 0);
             memcpy(put_log_record, &log_record, sizeof(log_record_t));
             HASH_ADD_INT(losers, trx_id, put_log_record);
        }
        else if(log_record.type == COMMIT || log_record.type == ROLLBACK){
            log_record_t* winner_log_record;
            HASH_FIND_INT(losers, &(log_record.trx_id), winner_log_record);
            HASH_ADD_INT(winners, trx_id, winner_log_record);     //winner 리스트에 추가
            HASH_DEL(losers, winner_log_record);              //loser 리스트에서 삭제
        }
        index = index + log_record.log_size;
    }
    //sort_trx();
    log_record_t* s;
    printf("Winner: ");
    for(s = winners; s != NULL; s = s->hh.next){
        fprintf(log_out_fp, "%d ",s->trx_id);
        printf("%d ", s->trx_id);
    }
    printf("\n");
    printf("Loser: ");
    for(s = losers; s != NULL; s = s->hh.next){
        fprintf(log_out_fp, "%d ",s->trx_id);
        printf("%d ", s->trx_id);
    }
    printf("\n");
}


void redo( int log_num){
    log_record_t log_record;
    int index = 0;
    while(log_read_record(log_fd, index, &log_record)){
        if(log_record.type == BEGIN){
            fprintf(log_out_fp, "LSN %ld [BEGIN] trx id: %d", log_record.LSN, log_record.trx_id);
            printf("LSN %ld [BEGIN] trx id: %d", log_record.LSN, log_record.trx_id);
        }
        else if(log_record.type == COMMIT){
            fprintf(log_out_fp, "LSN %ld [COMMIT] trx id: %d", log_record.LSN, log_record.trx_id);
            printf("LSN %ld [COMMIT] trx id: %d", log_record.LSN, log_record.trx_id);
        }
        else if(log_record.type == ROLLBACK){
            fprintf(log_out_fp, "LSN %ld [ROLLBACK] trx id: %d", log_record.LSN, log_record.trx_id);
            printf("LSN %ld [ROLLBACK] trx id: %d", log_record.LSN, log_record.trx_id);
        }
        else if(log_record.type == UPDATE || log_record.type == COMPENSATE){
            page_t* page = (page_t*)malloc(sizeof(page_t));
            int index =buf_read_frame(log_record.tid, log_record.pnum, page);
            if(log_record.LSN > page->leaf_page.pageLSN){
                int val_index = (log_record.LSN - 128) / 128;
                strcpy(page->leaf_page.records[val_index].value, log_record.new_image);
                fprintf(log_out_fp, "LSN %ld [UPDATE] trx id: %d", log_record.LSN, log_record.trx_id);
                printf("LSN %ld [UPDATE] trx id: %d", log_record.LSN, log_record.trx_id);
                buf_write_frame(index, log_record.tid, log_record.pnum, page);
            }
            else{
                fprintf(log_out_fp, "LSN %ld [CONSISER-REDO] trx id: %d", log_record.LSN, log_record.trx_id);
                printf("LSN %ld [CONSIDER-REDO] trx id: %d", log_record.LSN, log_record.trx_id);
            }
        }
        log_record_t* log = (log_record_t*)malloc(sizeof(log_record));
        memcpy(log, &log_record, sizeof(log_record_t));
        HASH_ADD_INT(all, LSN, log);
        last_record = log;
        index = index + log_record.log_size;
    }
    current_LSN = last_record->LSN + last_record->log_size;
    prev_LSN = last_record->LSN;
    int end_index = index;
}

void undo( int log_num){
    
   for(log_record_t* check_record = last_record; check_record != NULL; check_record = check_record->hh.prev){
        log_record_t* find;
        HASH_FIND_INT(losers, &(check_record->trx_id), find);
        if(find == NULL) continue;
        if(check_record->type != UPDATE) continue;
        page_t* page = (page_t*)malloc(sizeof(page_t));
        buf_read_frame(check_record->tid, check_record->pnum, page);
        int val_index = (check_record->LSN - 128) / 128;
        strcpy(page->leaf_page.records[val_index].value, check_record->old_image);
        fprintf(log_out_fp, "LSN %ld [CLR] trx id: %d", check_record->LSN, check_record->trx_id);
        printf("LSN %ld [CLR] trx id: %d", check_record->LSN, check_record->trx_id);
        /*
        CLR log 발급
        */
        log_record_t CLR;
        CLR.log_size = CLR_SIZE;
        CLR.LSN = current_LSN;
        CLR.prev_LSN = prev_LSN;
        CLR.trx_id = check_record->trx_id;
        CLR.type = COMPENSATE;
        CLR.tid = check_record->tid;
        CLR.pnum = check_record->pnum;
        CLR.offset = check_record->offset;
        CLR.data_length = check_record->data_length;
        strcpy(CLR.old_image, check_record->new_image);
        strcpy(CLR.new_image, check_record->old_image);
        /* CLR.next  = ... */
        log_buf[current_log_index] = CLR;
        current_log_index = (current_log_index + 1) % LOG_SIZE;
        prev_LSN = current_LSN;
        current_LSN = current_LSN + CLR_SIZE;
   }

}
int log_read_record(int fd, int LSN, log_record_t* log_record){

    int size;
    int exist = pread(fd, &size, sizeof(int), LSN);

    if(exist <= 0){
        printf("end\n");
        return 0;
    }

    int success = pread(fd, log_record, size, LSN);
    
    if(success <= 0){
        printf("error\n");
        return 0;
    }

    return 1;
}

void make_update_log(int trx_id, int tid, uint64_t pnum, int index, char* old_value, char* new_value){
    log_record_t log;
    log.log_size = UPDATE_SIZE;
    log.LSN = current_LSN;
    log.prev_LSN = prev_LSN;
    log.trx_id = trx_id;
    log.type = UPDATE;
    log.tid = tid;
    log.pnum = pnum;
    log.data_length = 120;
    strcpy(log.old_image, old_value);
    strcpy(log.new_image, new_value);
    /*버퍼에 추가 후 변수 변경*/
    log_buf[current_log_index] = log;
    current_log_index = (current_log_index + 1) % LOG_SIZE;
    if(current_log_index == flushed_index){
        flush_log();
    }
    /*LSN 관리*/
    prev_LSN = current_LSN;
    current_LSN = current_LSN + UPDATE_SIZE;

}

void make_begin_log(int trx_id){
    pthread_mutex_lock(&log_buf_latch);
    log_record_t log;
    log.log_size = BEGIN_SIZE;
    log.LSN = current_LSN;
    log.prev_LSN = prev_LSN;
    log.trx_id = trx_id;
    log.type = BEGIN;

    log_buf[current_log_index] = log;
    current_log_index = (current_log_index + 1) % LOG_SIZE;
    if(current_log_index == flushed_index){
        flush_log();
    }
    prev_LSN = current_LSN;
    current_LSN = current_LSN + BEGIN_SIZE;
    pthread_mutex_unlock(&log_buf_latch);
}

void make_commit_log(int trx_id){
    pthread_mutex_lock(&log_buf_latch);
    log_record_t log;
    log.log_size = COMMIT_SIZE;
    log.LSN = current_LSN;
    log.prev_LSN = prev_LSN;
    log.trx_id = trx_id;
    log.type = COMMIT;
    
    log_buf[current_log_index] = log;
    current_log_index = (current_log_index + 1) % LOG_SIZE;
    if(current_log_index == flushed_index){
        flush_log();
    }
    prev_LSN = current_LSN;
    current_LSN = current_LSN + COMMIT_SIZE;
    pthread_mutex_unlock(&log_buf_latch);
}

void make_rollback_log(int trx_id){
    pthread_mutex_lock(&log_buf_latch);
    log_record_t log;
    log.log_size = ROLLBACK_SIZE;
    log.LSN = current_LSN;
    log.prev_LSN = prev_LSN;
    log.trx_id = trx_id;
    log.type = ROLLBACK;
    
    log_buf[current_log_index] = log;
    current_log_index = (current_log_index + 1) % LOG_SIZE;
    if(current_log_index == flushed_index){
        flush_log();
    }
    prev_LSN = current_LSN;
    current_LSN = current_LSN + ROLLBACK_SIZE;
    pthread_mutex_unlock(&log_buf_latch);
}



void flush_log(){
    
    while((flushed_index + 1) % LOG_SIZE != current_log_index){
        flushed_index = (flushed_index + 1) % LOG_SIZE;
        
        pwrite(log_fd, &log_buf[flushed_index], log_buf[flushed_index].log_size, log_buf[flushed_index].LSN); 
    }

}