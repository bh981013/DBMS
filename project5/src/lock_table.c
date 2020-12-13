#include "lock_table.h"




node_t* hash_table;





int
init_lock_table()
{	
	hash_table = NULL;
	pthread_mutex_init(&lock_table_latch, 0);
	/* DO IMPLEMENT YOUR ART !!!!! */

	return 0;
}

void print_node(node_t* node){/*
	printf("\n\n\n\n");
	printf("-----------------------------------------\n");
	printf("노드정보) key:%ld, head ID: %d, tail ID: %d\n", node->key.record_id, node->head->trx_id, node->tail->trx_id );
	printf("\n");
	lock_t* lock = node->head;
	while(lock != NULL){
	printf("lock정보: trx_id: %d, lock_mode: %d \n", lock->trx_id, lock->lock_mode);
	lock = lock->prev;
	printf("\t\t\tV\t\t\n");
	}
	printf("-----------------------------------\n\n\n\n");*/
}

int i = 1;
void my_abort(int trx_id){
	//pthread_mutex_lock(&lock_table_latch);
	printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
	
	printf("전체 중%d번째\n trx:%d abort()\n", i, trx_id);
	i++;
	trx_t* trx;
	HASH_FIND_INT(get_trx_table(), &trx_id, trx);
	lock_t* lock = trx->lock;
	val_t* val_str;
	node_t* node;
	page_t page;
	while(lock != NULL){
		node = lock->sent;
		print_node(node);
		printf("하나씩 없애볼까?\n");
		if(lock->next == NULL && lock->lock_mode == 1){
			printf("바꾼거 되돌리기\n");
			val_str = trx->old_val;
			//printf("val_str_p: %p\n", val_str);
			//printf("tid: %d, pnum: %ld\n", val_str->tid, val_str->pnum);
	
			
			printf("1...\n");
			int index = buf_read_frame(val_str->tid, val_str->pnum, &page);
			printf("2...\n");
			strcpy(page.leaf_page.records[val_str->index].value, val_str->val);
			buf_write_frame(index, val_str->tid, val_str->pnum, &page);
			printf("3..\n");
			my_unlock(val_str->index);
			trx->old_val = val_str->next;
		}
		printf("abort후 노드의 모습:\n");
		print_node(node);
		lock = lock->trx_next;
	}
	//trx->lock = NULL;
	/*for(int j = 0; j<500; j++){
		arr[trx_id%500][j] = 0;
	}
    memset(arr[trx_id%500], 0, sizeof(arr[0]));*/
	printf("trx_id: %d abort done\n", trx_id);

	
	//pthread_mutex_unlock(&lock_table_latch);
}

int
lock_acquire(int table_id, int64_t key, int trx_id, int lock_mode, lock_t* lock)
{
	//lock occurs
	//printf("trxid: %d lock aquire시도\n", trx_id);
	pthread_mutex_lock(&lock_table_latch);
	//printf("\ntrxid: %d 가 lock을 이을차례\n", trx_id);
	//node pointer points hash table entry
	node_t* node;

	//makes lock
	//lock = (lock_t*)malloc(sizeof(lock_t));
	lock->trx_next = NULL;
	//make temp_key to find
	keys_t* temp_key = (keys_t*)malloc(sizeof(keys_t));

	memset(temp_key, 0, sizeof(keys_t));
	temp_key->table_id = table_id;
	temp_key->record_id = key;

	//find table_id, record_id in hash table
	HASH_FIND(hh, hash_table, temp_key, sizeof(keys_t), node);
	
	//if there's no pred, return new lock_t	
	if(node == NULL){
		//printf("key:%ld -> trx_id: %d:, lock_mode: %d 내가 1빠!\n", key, trx_id, lock_mode);
		node = (node_t*)malloc(sizeof(node_t));
		memset(node, 0, sizeof(node_t));

		node->key.table_id = table_id;		
		node->key.record_id = key;
		
		lock-> prev = NULL;
		lock-> next = NULL;
		//printf("lock next NULL\n");
		lock-> sent = node;

		lock->trx_id = trx_id;
		lock->lock_mode = lock_mode;

		trx_t* trx;
		HASH_FIND_INT(get_trx_table(), &trx_id, trx);
		//printf("trx id: %d\n", trx->trx_id);
		lock_t* trx_lock = trx->lock;
		
		if(trx_lock == NULL){
			trx->lock = lock;
			trx->lock->trx_next = NULL;
			//printf("trx의 lock이 처음 이어짐\n");
		}
		else{
			while(trx_lock->trx_next != NULL){
				trx_lock = trx_lock->trx_next;
				//printf("찾는중.. \n");
			}
			trx_lock->trx_next = lock;
			trx_lock->trx_next->trx_next = NULL;
		}

		node->tail = lock;
		node->head = lock;
		HASH_ADD(hh, hash_table, key, sizeof(keys_t), node);
		//unlock before returning
		//printf("노드의 첫 lock을 이었따!\n");	
		pthread_mutex_unlock(&lock_table_latch);
		
		return 0;
	}

	//if exists
	lock->prev = NULL;	
	lock->sent = node;
	
	//printf("노드의 헤드의 trxid: %d, 내 trx_id: %d\n", node->head->trx_id, trx_id);
	//if there's pred, sleep until pred wakes up.
	if(node->head != NULL){
		//printf("진행\n");
		//print_node(node);
		//printf("key:%ld -> trx_id: %d:, lock_mode: %d 뒤에라도 서야지\n", key, trx_id, lock_mode);
		/*case: if I read or write into same record*/

		lock_t* prelock;
		//만약 달려있는 lock이 내가 매단거고 유일하면 그 lock을 return
		if(node->head->trx_id == node->tail->trx_id && node->head->trx_id == trx_id){
			pthread_mutex_unlock(&lock_table_latch);
			//printf("lock을 다아아아아이었따!\n");	
			return 0; //lock 획득
		}
		//if there exists lock which has same trx-> abort or continue
		else if((prelock = find_same_trx(node, trx_id)) != NULL){
			//if prelock is S and I am X
			if(prelock->lock_mode == 0 && lock_mode == 1){
				
				//printf("앞에 내가 있어! 모드도 나는 쓰는데 앞은 읽어!\n");
				//printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
				//my_abort(trx_id);
				pthread_mutex_unlock(&lock_table_latch);
				return 2;	//deadlock 발생!!
			}
			else
			{//prelock is already working, I also work
				//memcpy(ret_lock, lock, sizeof(lock_t));		
				pthread_mutex_unlock(&lock_table_latch);
				//printf("lock을 다아아아아이었따!\n");	
				return 0;
			} 
		}


		/*case: first time to read or write*/
		
		//before connection, deadlock이 일어날지 check
		//print_node(node);
		
		
		//printf("내가없으니 그냥 연결해볼까?\n");
		lock->trx_id = trx_id;
		lock->lock_mode = lock_mode;

		trx_t* trx;
		HASH_FIND_INT(get_trx_table(), &trx_id, trx);
		lock_t* trx_lock = trx->lock;
		if(trx_lock == NULL) {
			trx->lock = lock;
			trx->lock->trx_next = NULL;
		}
		else{
			while(trx_lock->trx_next != NULL){
				trx_lock = trx_lock->trx_next;
			}
			trx_lock->trx_next = lock;
			trx_lock->trx_next->trx_next = NULL;
		}
		
		//printf("연결완료\n");
		lock->next = node->tail;
		lock->prev = NULL;
		node->tail->prev = lock;
		node->tail = lock;
		lock_t* pre_lock;
		//만약 내 앞에 shared 밖에없고 내가 shared면
		if(lock_mode == 0){
			//printf("일단 나는 s 모드\n");
			pre_lock = lock->next;
			while(pre_lock->lock_mode == 0 ){
				//printf("prelock 의 trxid: %d", prelock->trx_id);
				pre_lock = pre_lock->next;
				if(pre_lock == NULL){
					//printf("null 까지 찾았다!\n");
					break;
				} 
			}
			if(pre_lock == NULL){
				printf("key:%ld -> trx_id: %d:, lock_mode: %d 인데 내앞 싹다 shared\n", key, trx_id, lock_mode);	
				print_node(node);
				pthread_mutex_unlock(&lock_table_latch);
				return 0;
			} 
		}	
		print_node(node);
		if(lock_mode == 0){
			//printf("나는 Smode이고,  첫 xmode 는 trx:%d, mode:%d\n", pre_lock->trx_id, pre_lock->lock_mode);
			arr[trx_id%500][(pre_lock->trx_id)%500] = 1;
		}
		else if(lock_mode == 1){
			lock_t* next_lock = lock->next;
			if(next_lock->lock_mode == 1){
				arr[trx_id%500][(next_lock->trx_id)%500] = 1;
			}
			else{
				while(next_lock->lock_mode == 0){
					arr[trx_id%500][(next_lock->trx_id)%500] = 1;
					next_lock = next_lock->next;
					if(next_lock == NULL) break;
				}
			}
		}
		int exist = detect(node, trx_id);
		if(exist == -1){
			printf("abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!abort발생!!\n");
			//my_abort(trx_id);
			node->tail = lock->next;
			lock->next->prev = NULL;
			
			lock_t* trx_lock2 = trx->lock;
			if(trx_lock2 == lock) trx->lock = NULL;
			else{
				while(trx_lock2->trx_next != lock){
					trx_lock2 = trx_lock2->trx_next;
				}
				trx_lock2->trx_next = NULL;
			}

			pthread_mutex_unlock(&lock_table_latch);
			return 2; //deadlock  발생!
		}

		pthread_mutex_lock(get_trx_table_latch());
		//printf("여기?11\n");
		pthread_mutex_lock(&(trx->trx_latch));
		//printf("아님요기??22\n");
		//ret_lock = lock;
		
		pthread_mutex_unlock(get_trx_table_latch());
		//printf("여기333\n");
		pthread_mutex_unlock(&lock_table_latch);
		//printf("trx: %d lock_aquire 종료\n", trx_id);
		return 1;
	}
}

void lock_wait(lock_t* lock_obj){//
	
	int trx_id = lock_obj->trx_id;
	trx_t* trx;
	HASH_FIND_INT(get_trx_table(), &trx_id, trx);
	pthread_cond_init(&(lock_obj->cond), NULL);
	printf("trx: %d 잠에듬..\n", lock_obj->trx_id);
	pthread_cond_wait(&(lock_obj->cond), &(trx->trx_latch));
	memset(arr[trx_id%100], 0, sizeof(int)* 100);
	printf("trx: %d 일어났다!\n", trx_id);
	//pthread_mutex_lock(&lock_table_latch);	//수정해야 할 수도있음.
	pthread_mutex_unlock(&(trx->trx_latch));
	//printf("일어났을때의 모습: \n");	
	print_node(lock_obj->sent);
	if(lock_obj->prev != NULL && lock_obj->prev->lock_mode == 0 && lock_obj->lock_mode == 0){
		//printf("뒤친구도 깨우자\n");
		trx_t* trx_next;
		HASH_FIND_INT(get_trx_table(), &(lock_obj->prev->trx_id), trx_next);
		//printf("000 trx ; %d\n", trx_id);
		//printf("1111 trx; %d\n", trx_id);
		pthread_mutex_lock(&(trx_next->trx_latch));
		//printf("2222 trx; %d\n", trx_id);
		pthread_cond_signal(&(lock_obj->prev->cond));
		//printf("333 trx; %d\n", trx_id);
		pthread_mutex_unlock(&(trx_next->trx_latch));
		//printf("444s trx; %d\n", trx_id);

	}
	
	//pthread_mutex_unlock(&lock_table_latch);	//이걸 잠구고 푸는게 맞는건가..?
	//printf("lock을 다아아아아이었따!\n");	
	return;

}



lock_t* find_same_trx(node_t* node, int trx_id){
	lock_t* lock = node->tail;
	while(lock != NULL){
		if(lock->trx_id == trx_id) return lock;
		lock = lock->next;
	}
	return NULL; 
}

int global_time = 0;
int DFS_visit(int u){
	trx_info[u].color = 1;
	trx_info[u].time1 == ++global_time;
	for(int i = 0; i<500; i++){
		if(arr[u][i] == 1){
			if(trx_info[i].color == 0){
				if(DFS_visit(i)== -1) return -1;
			}
			else{
				if(trx_info[i].time2 == 0){
					//printf("trx: %d와 trx; %d deadlock!\n", u, i);
					return -1;
				}
			}
		}
	}
	trx_info[u].time2 = ++global_time;
	return 0;
}

int DFS(){
	for(int i = 0; i < 500; i++){
		if(trx_info[i].color == 0){
			if(DFS_visit(i) == -1) return -1;
		}
	}
	return 0;
}

int detect(node_t* node, int trx_id){
	memset(trx_info, 0, sizeof(trx_info));
	int ret = DFS();
	if(ret == -1) return -1;

	//printf("확인끝!!\n");
	
	return 0; //not exist
}


int
lock_release(lock_t* lock_obj)
{
	//pthread_mutex_lock(&lock_table_latch);
	//printf("\n\n\n\n\ntrx_id:%d의 release 진행\n", lock_obj->trx_id);
	node_t* node = lock_obj->sent;
	//앞에 뭐가있고 뒤에없으면, 누군가는 아직 일하고있는중임.
	if(lock_obj->next != NULL && lock_obj->prev == NULL){
		printf("case1\n");
		lock_obj->next->prev = NULL;
		node->tail = lock_obj->next;
	}
	//앞에 뭐가있고 뒤에도 있으면 잎에는 일하고있으니까 뒤가 누구든지 깨우면 안됨.
	else if(lock_obj->next != NULL && lock_obj->prev !=NULL){
		printf("case2\n");
		//printf("node 의 주소:%p", node);
		//printf("lock next id: %d lock prev id: %d\n", lock_obj->next->trx_id, lock_obj->prev->trx_id);
		lock_obj->prev->next = lock_obj->next;
		lock_obj->next->prev = lock_obj->prev;
	}
	//앞에 뭐가없고 뒤에 있으면 뒤에는 일하고있으면 깨워도 아무일없고 일안하고있으면 내가 마지막이므로 깨워야댐
	else if(lock_obj->next == NULL && lock_obj->prev != NULL){	
		printf("case3:뒤에 꺠움\n");
		node->head = lock_obj->prev;
		lock_obj->prev->next = NULL;
		trx_t* prev_trx;
		HASH_FIND_INT(get_trx_table(), &(lock_obj->prev->trx_id), prev_trx);
		printf("깨워야될 trx: %d, key: %ld\n", prev_trx->trx_id, lock_obj->prev->sent->key.record_id);
		pthread_mutex_lock(&(prev_trx->trx_latch));
		pthread_cond_signal(&(lock_obj->prev->cond));
		pthread_mutex_unlock(&(prev_trx->trx_latch));
	}
	//앞뒤 둘다없으면 해당 노드를 해시테이블에서 삭제
	else{
		printf("case4:\n");
		printf("node 의 주소:%p " , node);
		HASH_DEL(hash_table, node);
	}
	print_node(node);
	//free(lock_obj);
	printf("lock_obj 의 주소:%p\n", lock_obj);
	printf("trx_id: %d의 release done\n", lock_obj->trx_id);

	return 0;
	/* GOOD LUCK !!! */
	
}
