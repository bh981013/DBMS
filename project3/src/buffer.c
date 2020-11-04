#include "buffer.h"
/*
void print_buf(){
	printf("-----------------------------------------ㄱㄱㄱㄱ\n");
	printf("버퍼 정보: \n");
	printf("buf info=> buf_size: %d, use_size: %d,\n LRU old: %d, LRU new: %d\n\n", buf_info.buf_size, buf_info.use_size, buf_info.LRU_old, buf_info.LRU_new);
		for(int i = 0; i<buf_info.use_size; i++){
		printf("pid: %d, pagenum: %ld\n",buf_arr[i].table_id, buf_arr[i].pagenum);
		printf("가르키는 old index: %d, new_index: %d\n", buf_arr[i].older_index, buf_arr[i].newer_index);
		
		printf("is_pinned: %d, is_dirty: %d\n\n", buf_arr[i].is_pinned, buf_arr[i].is_dirty);
		}	
	printf("ㄴㄴㄴㄴ-----------------------------------------\n");
}*/

int flush_page(int table_id, pagenum_t pagenum){
	//printf("\nflush_page()\n");
	for(int i= 0; i < buf_info.use_size; i++){
		if(buf_arr[i].pagenum == pagenum && buf_arr[i].table_id == table_id){
			file_write_page(buf_arr[i].table_id, buf_arr[i].pagenum, &(buf_arr[i].frame));
			buf_arr[i].is_dirty = 0;		
		}
	}
	//print_buf();
	return -1;
}

int flush_file(int table_id){
	for(int i = 0; i<buf_info.use_size; i++){
		if(buf_arr[i].table_id == table_id && buf_arr[i].is_dirty == 1){
			file_write_page(table_id, buf_arr[i].pagenum, &(buf_arr[i].frame));
			buf_arr[i].is_dirty = 0;
		//	printf("%d번째 buf를 flush했다\n", i);
			
		}
	}
	table_info[table_id].is_open = 0;
	//printf("%d번째 테이블을 전부 flush/close했다!!\n", table_id);
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
	return buf_arr;
}


int buf_find_frame(int table_id, pagenum_t pagenum){
	
	//printf("\nbuf_find_frame()\n");
	//printf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
	int i;
	i = buf_info.LRU_new;
	while(i != -1){
	//	printf("%d번째 index를 buffer에서 확인\n", i);
		if(buf_arr[i].pagenum == pagenum && buf_arr[i].table_id == table_id){
	//	printf("%d번째 버퍼에 tableid:%d  %ld페이지넘이 존재, find종료\n\n", i, table_id, pagenum);
	//	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
		return i;
		}		
		else {
			i = buf_arr[i].older_index;
			
		}
	}
	//printf("버퍼에서 못찾음...\n");
	
	//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n\n");
	return -1;

}

int buf_read_frame(int table_id, pagenum_t pagenum, page_t* frame){
	//print_buf();	
	//printf("\nbuf_read()\n");
	//printf("ㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱㄱ");
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
		find_buf.is_pinned = 0;
		
		if(buf_info.use_size < buf_info.buf_size){
			buf_index = buf_info.use_size;		
			buf_info.use_size++; //버퍼가 안차있으면 usesize 증가
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
	buf_arr[buf_index].is_pinned = 1;
	//print_buf();
	//printf("ㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴㄴ\n\n");
	
	return buf_index;
}
//buf에서 페이지를 읽어왔을때 그 페이지를 변경하려면 pin하고 그렇지 않을거면 pin해줄필요 없음.

int buf_write_frame(int table_id, pagenum_t pagenum, page_t* frame){
	//printf("buf_write_frame()\n");
	int write_index;
	int buf_index = buf_find_frame(table_id, pagenum);
	if(buf_index == -1){
		//printf("%ld가 왜 buf에 없냐;;;누가 가져갔어\n", pagenum);
		//print_buf();
		return -1;
	}
	//쓸곳을 찾음....
	buf_arr[buf_index].frame = *frame;
	buf_arr[buf_index].table_id = table_id;
	buf_arr[buf_index].pagenum = pagenum;
	buf_arr[buf_index].is_dirty = 1;
	unpin(buf_index);
}
//buf에 페이지를 쓸때엔 페이지를 다 썼다는 표시이므로 함수내에서 unpin 해줘도 됨.

void unpin(int index){
	buf_arr[index].is_pinned = 0;
	
}
//unpin할시엔 해당 페이지에 대한 사용이끝났다는 의미이므로, LRU list에 넣어줌
void unpin_buf(int pid, pagenum_t pagenum){
	int index = buf_find_frame(pid, pagenum);
	if(index != -1)	buf_arr[index].is_pinned = 0;
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
pagenum_t buf_alloc_page(int table_id){
	//버퍼에 있는 헤더페이지 flush(없으면 안함)
	flush_page(table_id, header_page_num); //tid에 해당하는 header을 파일에 씀
	int index;
	
	pagenum_t pagenum = file_alloc_page(table_id); //???????........
	//printf("alloc한 pagenum: %ld\n",pagenum);	
	page_t hpage, fpage;
	file_read_page(table_id, header_page_num, &hpage);

	if(buf_find_frame(table_id, header_page_num) != -1){
	buf_write_frame(table_id, header_page_num, &hpage);
	}
	
	file_read_page(table_id, pagenum, &fpage);
	//printf("할당받은 freepage가 가르키는 pagenum: %ld\n", fpage.free_page.next_pnum);
	buf_str buf;
	buf.frame = fpage;
	buf.table_id = table_id;	
	buf.pagenum = pagenum;
	buf.is_pinned = 1;
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
	//print_buf();
	//printf("buf_alloc 종료\n\n");
	return pagenum;
}

void buf_free_page(int pid, pagenum_t pagenum){
	//printf("buf free page\n");
	page_t hpage;
	page_t fpage;
	buf_read_frame(pid, pagenum, &fpage);
	buf_read_frame(pid, header_page_num, &hpage);
	memset(&fpage, 0, sizeof(page_t));
	//printf("원래 헤더가 가르키던 freepnum: %ld\n", hpage.header_page.free_pnum);	
	fpage.free_page.next_pnum = hpage.header_page.free_pnum;
	hpage.header_page.free_pnum = pagenum;
	buf_write_frame(pid, header_page_num, &hpage);
	buf_write_frame(pid, pagenum, &fpage);
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
	while(buf_arr[index].is_pinned != 0){
	index = buf_arr[index].newer_index;
	}
	if(index == -1){
		printf("buffer에 모두 pin돼있다\n");
		return -1;
	}
	buf_str buf = buf_arr[index];
	if(buf.is_dirty == 1) file_write_page(buf.table_id, buf.pagenum, &(buf.frame));

	buf_arr[buf_arr[index].newer_index].older_index = -1;
	buf_info.LRU_old = buf_arr[index].newer_index;




	return index;
}	



