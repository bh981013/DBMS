#include "file.h"
#include "db.h"
/*
int check_path(char* pathname){
	for(int i = 0; i< sizeof(path_arr)/sizeof(char*); i++){
		if(strcmp(path_arr[i], pathname) == 0) return i + 1; 
	}
	return -1;

}*/


int open_file(char* pathname, int* is_made)
{
	//printf("open_file\n");
	page_t hpage;
	
	int fd = open(pathname, O_RDWR);
	//printf("fd:%d\n",fd);
	*is_made = 0;
	if(fd < 0){
		*is_made = 1;
		fd = open(pathname, O_RDWR|O_CREAT, 0644);
		//printf("fd:%d\n",fd);
		//printf("start new\n");
		//조정필요
	}
	return fd;
}


page_t* make_free_page(int n)
{
	page_t* page;
	page = (page_t*)malloc(n*sizeof(page_t));
	if(page== NULL){
		perror("free_page creation.");
		exit(EXIT_FAILURE);
	}
	return page;
}



pagenum_t file_alloc_page(int table_id){
	page_t* hpage = (page_t*)malloc(4096); //header page 객체 생성
	page_t fpage; //free page 객체 생성
	file_read_page(table_id, header_page_num, hpage); //file의 header page를 읽음
	uint64_t num = hpage->header_page.num_of_p; //header page 에서 num_of_p 읽음
	//printf("제에에발 1: %ld\n", num);
	if(hpage->header_page.free_pnum == 0)
	{
		page_t* f_page = make_free_page(1000);
		file_write_page(table_id, num, &f_page[0]);
		for(pagenum_t i = 1; i <= 999; i++)
		{
			f_page[i].free_page.next_pnum = num + i - 1;
			//printf("%ld: %ld\n",i, fpage[i].free_page.next_pnum);
			file_write_page(table_id, num+i, &f_page[i]);
			//printf("대입값:%ld\n", f_page[i].free_page.next_pnum);
		}
		f_page[0].free_page.next_pnum = 0;

		hpage->header_page.free_pnum = num + 999;
		hpage->header_page.num_of_p += 1000;
	}
	pagenum_t free_num = hpage->header_page.free_pnum; 	//header page가 가르키고 있는 free page number을 가져옴
	//printf("전에 헤더가 가르키던 freepnum:%ld\n", free_num);
	file_read_page(table_id, free_num, &fpage); 			//그 free page를 읽음
	//printf("%ld가 가르키던 next_pnum: %ld\n", free_num, fpage.free_page.next_pnum);
	hpage->header_page.free_pnum = fpage.free_page.next_pnum; //header page가 다음 free_page를 가르키게 함
	file_write_page(table_id, header_page_num, hpage);
	return free_num;
	free(hpage);	
}

void file_free_page(int table_id, pagenum_t pagenum){
	page_t fpage;
	page_t hpage;
	file_read_page(table_id, header_page_num, &hpage);
	pagenum_t before_fnum = hpage.header_page.free_pnum; //기존에 header_page가 가르키던 free page num을 before_fnum이라는 변수에 저장
	hpage.header_page.free_pnum = pagenum;			//header page의 free page num이 새로운 pagenum을 가르키게함
	fpage.free_page.next_pnum = before_fnum;		//생성한 free page가 before_fnum을 가르키게함
	file_write_page(table_id, header_page_num, &hpage); 
	file_write_page(table_id, pagenum, &fpage);			//freepage 와 header page를 write함
	//printf("헤더가 가르키고있는 free_page: %ld\n", hpage.header_page.free_pnum);
}

void file_read_page(int table_id, pagenum_t pagenum, page_t* dest){
	pagenum = pagenum * page_size;
	int fd = table_info[table_id].fd;
	int a = pread(fd, dest, page_size, pagenum);
	if(a == -1) perror("read error");
	//printf("읽은 위치 %d바이트\n", b);
	//(*dest).header_page.free_pnum = 123;
}

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src){
	pagenum = pagenum * page_size;	
	int fd = table_info[table_id].fd;	
	int write_test = pwrite(fd, src, page_size, pagenum);
	if(fdatasync(fd) == -1){ 
	printf("sync error");
	}
	if(write_test == -1) perror("write error");

}






