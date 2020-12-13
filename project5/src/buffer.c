#include "buffer.h"

void print_buf(){
	printf("-----------------------------------------ㄱㄱㄱㄱ\n");
	printf("버퍼 정보: \n");
	printf("buf info=> buf_size: %d, use_size: %d,\n LRU old: %d, LRU new: %d\n\n", buf_info.buf_size, buf_info.use_size, buf_info.LRU_old, buf_info.LRU_new);
		for(int i = 0; i<buf_info.use_size; i++){
			char a[10];
			printf("pid: %d, pagenum: %ld\n",buf_arr[i].table_id, buf_arr[i].pagenum);
			printf("가르키는 old index: %d, new_index: %d\n", buf_arr[i].older_index, buf_arr[i].newer_index);
			int k = pthread_mutex_trylock(&(buf_arr[i].page_latch));
			printf("이건가?\n");
			if(k == 0){
			pthread_mutex_unlock(&buf_arr[i].page_latch);	
			strcpy(a, "unlocked");
			} 
			else strcpy(a, "locked");
			printf("is_locked: %s, is_dirty: %d\n\n",a, buf_arr[i].is_dirty);
		}	
	printf("ㄴㄴㄴㄴ-----------------------------------------\n");
}

int flush_page(int table_id, pagenum_t pagenum){
	//printf("\nflush_page()\n");
	pthread_mutex_lock(&buf_info.buf_latch);
	for(int i= 0; i < buf_info.use_size; i++){
		if(buf_arr[i].pagenum == pagenum && buf_arr[i].table_id == table_id){
			file_write_page(buf_arr[i].table_id, buf_arr[i].pagenum, &(buf_arr[i].frame));
			buf_arr[i].is_dirty = 0;		
		}
	}
	pthread_mutex_unlock(&buf_info.buf_latch);
	//print_buf();
	return -1;
}

int flush_file(int table_id){
	pthread_mutex_lock(&buf_info.buf_latch);
	for(int i = 0; i<buf_info.use_size; i++){
		if(buf_arr[i].table_id == table_id && buf_arr[i].is_dirty == 1){
			file_write_page(table_id, buf_arr[i].pagenum, &(buf_arr[i].frame));
			buf_arr[i].is_dirty = 0;
			//printf("%d번째 buf를 flush했다\n", i);
			
		}
	}
	table_info[table_id].is_open = 0;
	//printf("%d번째 테이블을 전부 flush/close했다!!\n", table_id);
	pthread_mutex_unlock(&buf_info.buf_latch);
	return table_id;
}



buf_str* make_buf(int num){
	int buf_size = num;
	buf_arr = (buf_str*)malloc(sizeof(buf_str) * buf_size);
	for(int i = 0; i< num; i++){
		buf_arr[i].is_dirty = 0;
	}
	buf_info.buf_size = num;
	buf_info.use_size = 0;
	buf_info.LRU_new = -1;
	buf_info.LRU_old = 0;
	pthread_mutex_init(&buf_info.buf_latch, 0);
	//printf("버퍼만듬\n");
	return buf_arr;
}


int buf_find_frame(int table_id, pagenum_t pagenum){
	
	//printf("\nbuf_find_frame()\n");
	//printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	int i = 0;
	while(i != buf_info.use_size){
		if(buf_arr[i].pagenum == pagenum && buf_arr[i].table_id == table_id){
		return i;
		}		
		else {
			i++;
		}
		
	}
	printf("버퍼에서 못찾음...\n");
	
	//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
	

	return -1;

}
int qwe = 1;
int buf_read_frame(int table_id, pagenum_t pagenum, page_t* frame){
	//print_buf();	
	
	//printf("ㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱ");
	pthread_mutex_lock(&buf_info.buf_latch);
	//printf("\ntid:%d의 pnum: %ld를 buf_read()\n", table_id, pagenum);
	//printf("%d번쩨\n", qwe);
	qwe++;
	//printf("buf_latch 잠구겠습니다-");
	buf_str find_buf;
	buf_str evict_buf;
	int evict_index = buf_info.use_size; //사용중인 버퍼의 맨끝으로 들어가야할 index를 초기화
	int buf_index = buf_find_frame(table_id, pagenum);
	if(buf_index == -1) //buf에 해당하는 pagenum이 없다면
	{
		//printf("버퍼에 없어서 파일에서 찾아서 준다\n");
		file_read_page(table_id, pagenum, &(find_buf.frame)); 
		//find_buf의 frame에 원하는 page저장
		find_buf.table_id = table_id;
		find_buf.pagenum = pagenum;
		find_buf.is_dirty = 0;
		
		if(buf_info.use_size < buf_info.buf_size){
			buf_index = buf_info.use_size;
			buf_info.use_size++; //버퍼가 안차있으면 usesize 증가
			//printf("현재 use size: %d\n", buf_info.use_size);
		}	
		else if(buf_info.use_size == buf_info.buf_size)  //만약 버퍼가 꽉차있으면, evict_index를 찾아서 차출
		{
			//printf("버퍼가 꽉차서 좀 빼야겠네...\n");
			buf_index = evict_and_alloc();
			//차출해야될 buffer의 index를 찾음
		}

		if(buf_info.LRU_new != -1){
		buf_arr[buf_info.LRU_new].newer_index = buf_index;		
		}		
		find_buf.older_index = buf_info.LRU_new;
		find_buf.newer_index = -1;
		buf_info.LRU_new = buf_index;
		
		buf_arr[buf_index] = find_buf;
		pthread_mutex_init(&buf_arr[buf_index].page_latch, 0);
		//찾은 buffer을 차출한 buffer 자리에 씀.
		*frame  = find_buf.frame;
		//printf("버퍼의 %d 자리에 pagenum: %ld를 썼다, 그 후 frame을 read함, buf_read 종료\n\n", evict_index, pagenum);
	}
	else{
		*frame = buf_arr[buf_index].frame;
		//print_buf();
		//printf("LRU list 조정\n");	
		//printf("buf_index:%d\n", buf_index);
		
		int new = buf_arr[buf_index].newer_index;
		int old = buf_arr[buf_index].older_index;
		//printf("%d번째 버퍼가 가르키는 new:%d old:%d\n", buf_index, new, old);	
		if(new != -1){	
			buf_arr[new].older_index = old;  
			if(old != -1) buf_arr[old].newer_index = new;
			buf_arr[buf_info.LRU_new].newer_index = buf_index;		
			buf_arr[buf_index].older_index = buf_info.LRU_new;
			if(buf_index == buf_info.LRU_old) buf_info.LRU_old = buf_arr[buf_index].newer_index;
			buf_arr[buf_index].newer_index = -1;
			buf_info.LRU_new = buf_index;
			
		}
		//printf("버퍼의 %d 번째에서 pagenum: %ld을 frame에 read\n", buf_index, pagenum);	
	}
	 //printf("여기서멈추면 ㅇㅈ\n");
	pthread_mutex_lock(&buf_arr[buf_index].page_latch);
	//print_buf();
	//printf("ㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴ\n\n");
	//printf("-buf_latch 풀겠습니다\n");
	pthread_mutex_unlock(&buf_info.buf_latch);
	//printf("\n%d의 %ld를 buf_read 종료\n", table_id, pagenum);
	return buf_index;
}
//buf에서 페이지를 읽어왔을때 그 페이지를 변경하려면 pin하고 그렇지 않을거면 pin해줄필요 없음.
pthread_mutex_t write_mutex = PTHREAD_MUTEX_INITIALIZER;
int buf_write_frame(int buf_index, int table_id, pagenum_t pagenum, page_t* frame){
	pthread_mutex_lock(&write_mutex);
	//printf("buf_write_frame()\n");
	//int write_index;
	//pthread_mutex_lock(&buf_info.buf_latch);
	//int buf_index = buf_find_frame(table_id, pagenum);
	/*
	if(buf_index == -1){
		printf("error: %ld가 왜 buf에 없냐;;;누가 가져갔어\n", pagenum);
		print_buf();
		return -1;
	}*/
	//쓸곳을 찾음....
	buf_arr[buf_index].frame = *frame;
	buf_arr[buf_index].table_id = table_id;
	buf_arr[buf_index].pagenum = pagenum;
	buf_arr[buf_index].is_dirty = 1;
	
	
	pthread_mutex_unlock(&buf_arr[buf_index].page_latch);
	pthread_mutex_unlock(&write_mutex);
	//pthread_mutex_unlock(&buf_info.buf_latch);
	
}
//buf에 페이지를 쓸때엔 페이지를 다 썼다는 표시이므로 함수내에서 unpin 해줘도 됨.

//unpin할시엔 해당 페이지에 대한 사용이끝났다는 의미이므로, LRU list에 넣어줌

void my_unlock(int index){
	pthread_mutex_unlock(&buf_arr[index].page_latch);
}

/*
int file_to_buf(int table_id, pagenum_t pagenum){
	page_t hpage;
	file_read_page(table_id, header_page_num, &hpage);

	if(buf_find_frame(table_id, header_page_num) != -1){
	buf_write_frame(table_id, header_page_num);
	}
	else if(buf_find_frame(table_id, header_page_num) == -1){
		if(buf_info.use_size == buf_info.buf_size){
			int index = evict_and_alloc();
			buf_arr[index].frame = hpage;
			buf_arr[index].table_id = table_id;
			buf_arr[index].pagenum = pagenum;
			buf_arr[index].is_dirty = 0;
			buf_arr[index].is_pinned = 0;
			buf_arr[index].older_index
		}	

}*/
int buf_alloc_page(int table_id, pagenum_t* ret_num){
	//printf("buf_alloc_page()\n");
	//버퍼에 있는 헤더페이지 flush(없으면 안함)
	flush_page(table_id, header_page_num); //tid에 해당하는 header을 파일에 씀
	int index;
	
	pagenum_t pagenum = file_alloc_page(table_id); //???????........
	//printf("alloc한 pagenum: %ld\n",pagenum);	
	page_t hpage, fpage;
	file_read_page(table_id, header_page_num, &hpage);
	int hindex = buf_find_frame(table_id, header_page_num);
	if(hindex != -1){
	buf_write_frame(hindex, table_id, header_page_num, &hpage);
	}
	
	file_read_page(table_id, pagenum, &fpage);
	//printf("할당받은 freepage가 가르키는 pagenum: %ld\n", fpage.free_page.next_pnum);
	buf_str buf;
	buf.frame = fpage;
	buf.table_id = table_id;	
	buf.pagenum = pagenum;
	*ret_num = pagenum;
	buf.is_dirty = 0;
	buf.newer_index = -1;
	buf.older_index = buf_info.LRU_new;
	
	if(buf_info.buf_size > buf_info.use_size){
		//printf("buffer가 여유가 있네\n");
		
		if(buf_info.LRU_new != -1){
			buf_arr[buf_info.LRU_new].newer_index = buf_info.use_size;
		}
		buf_info.LRU_new = buf_info.use_size;		
		buf_arr[buf_info.use_size] = buf;
		index = buf_info.use_size;
		buf_info.use_size++;
	}
	else if(buf_info.buf_size == buf_info.use_size){
		//printf("alloc 할라했는데 buffer가 꽉찼구만~");
		index = evict_and_alloc();\
		
		buf.newer_index = -1;
		buf.older_index = buf_info.LRU_new;
		buf_arr[buf_info.LRU_new].newer_index = index;
		buf_info.LRU_new = index;

		buf_arr[index] = buf;
	}
	pthread_mutex_init(&buf_arr[index].page_latch, 0);
	pthread_mutex_lock(&(buf_arr[index].page_latch));
	//print_buf();
	//printf("buf_alloc 종료\n\n");
	return index;
}

void buf_free_page(int pid, pagenum_t pagenum){
	//printf("buf free page\n");
	page_t hpage;
	page_t fpage;
	int findex = buf_read_frame(pid, pagenum, &fpage);
	int hindex = buf_read_frame(pid, header_page_num, &hpage);
	memset(&fpage, 0, sizeof(page_t));
	//printf("원래 헤더가 가르키던 freepnum: %ld\n", hpage.header_page.free_pnum);	
	fpage.free_page.next_pnum = hpage.header_page.free_pnum;
	hpage.header_page.free_pnum = pagenum;
	buf_write_frame(hindex, pid, header_page_num, &hpage);
	buf_write_frame(findex, pid, pagenum, &fpage);
	//printf("이제 헤더가 가르키는 freepnum: %ld\n", hpage.header_page.free_pnum);
}



/*

int find_evict(){
	printf("\nfind_evict()\n");
	int index = buf_info.LRU_old;
	while(index == -1){
		if(buf_arr[index].is_pinned == 0)
		{	//index에서 pin이 돼있지 않다면,
			printf("%d를 evict해볼까? find_evict 종료\n", index);	
			return index;			//해당 index를 return
		}	
		else index = buf_arr[index].newer_index; //index 에서 pin이 돼있다면, newer index 찾음.
	}
	printf("evict 할 곳을 못찾았다.., find_evict 종료\n");
}*/

int evict_and_alloc(){ //buffer가 full 이라고 가정, buffer에서 evict 후 alloc함
	int index = buf_info.LRU_old;
	int is_locked;
	while(index != -1){
		is_locked = pthread_mutex_trylock(&buf_arr[index].page_latch);
		if(is_locked == 0){
			pthread_mutex_unlock(&buf_arr[index].page_latch);
			//printf("안잠겨있는곳 찾음\n");
			break;
		} 
		//printf("%d에 lock돼있네? 다음꺼: ", index);
		index = buf_arr[index].newer_index;
		printf("%d\n", index);
	}
	if(index == -1){
		//printf("buffer에 모두 lock돼있다\n");
		return -1;
	}
	buf_str buf = buf_arr[index];
	if(buf.is_dirty == 1) file_write_page(buf.table_id, buf.pagenum, &(buf.frame));

	buf_arr[buf_arr[index].newer_index].older_index = buf_arr[index].older_index;
	if(buf_arr[index].older_index != -1)
	 buf_arr[buf_arr[index].older_index].newer_index = buf_arr[index].newer_index;
	buf_info.LRU_old = buf_arr[index].newer_index;
	//printf("evict 성공\n");
	return index;
}	



