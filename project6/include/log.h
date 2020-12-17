#include "file.h"
#include "uthash.h"

#define WINNER 1
#define LOSER 0

#define BEGIN 0
#define UPDATE 1
#define COMMIT 2
#define ROLLBACK 3
#define COMPENSATE 4

#define LOG_SIZE 100000000

#define BEGIN_SIZE 28
#define COMMIT_SIZE 28
#define ROLLBACK_SIZE 28
#define UPDATE_SIZE 288
#define CLR_SIZE 296

typedef struct done_trx_t{
    int id;
    UT_hash_handle hh;
}done_trx_t;


typedef struct log_record_t{
    int log_size;
    uint64_t LSN;
    uint64_t prev_LSN;
    int trx_id;
    int type;
    int tid;
    int pnum;
    int offset;
    int data_length;
    char old_image[120];
    char new_image[120];
    int next;
    UT_hash_handle hh;
}log_record_t;

log_record_t* last_record;

int current_LSN;
int prev_LSN;

int current_log_index;
int flushed_index;

log_record_t log_buf[LOG_SIZE];
pthread_mutex_t log_buf_latch;


void init_log_buf();

int recover(int flag, int log_num);
int log_read_record(int fd, int LSN, log_record_t* log_record);
int help_sort_trx(done_trx_t* a, done_trx_t* b);
void sort_trx();

int log_read_record(int fd, int LSN, log_record_t* log_record);

void make_update_log(int trx_id, int tid, uint64_t pnum, int index, char* old_value, char* new_value);
void make_rollback_log(int trx_id);
void make_commit_log(int trx_id);
void make_begin_log(int trx_id);

void undo( int log_num);
void redo( int log_num);
void analysis();

void flush_log();