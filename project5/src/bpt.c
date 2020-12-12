
#define Version "1.14"

#include "bpt.h"

void init_index(int buf_num){
	make_buf(buf_num);
}


int find_table(char* pathname){
	for(int i = 1; i <= 10; i++){
	if(strcmp(table_info[i].pathname, pathname) == 0) return i;
	}
	return -1;
}

int open_data(char* pathname)
{	
	//printf("open data()\n");
	page_t hpage;
		
	int table_id = find_table(pathname);
	//printf("처음여는거면 -1: %d\n",table_id);
	//printf("table size: %d\n", table_info[0].size);
	if(table_id != -1){
		if(table_info[table_id].is_open == 1){
			//printf("already opened\n");
			return table_id;
		}
		else if(table_info[table_id].is_open == 0){
			//printf("existing table-> made_open\n");
			table_info[table_id].is_open = 1;
			return table_id;		
		}
	}
	else if(table_id == -1)
	{
		if(table_info[0].size >= 10){
		// printf("table full\n");
		return -1;
		}
	}
	//printf("열어볼까?\n");
	int is_made;
	int fd = open_file(pathname, &is_made);

	if(table_id == -1){
		table_info[0].size++;
		table_id = table_info[0].size;
		table_info[table_id].fd = fd;
		strcpy(table_info[table_id].pathname, pathname);
		table_info[table_id].is_open = 1;
		}
	if(is_made == 1){	
		page_t hpage;
		hpage.header_page.free_pnum = 0;
		hpage.header_page.root_pnum = 0;
		hpage.header_page.num_of_p = 1;
		file_write_page(table_id, header_page_num, &hpage);
		//printf("이제 테이블아이디:%d\n", table_id);
		}	
	
	return table_id;
}


page_t get_header_page(int pid){
	//printf("\nget header page()\n");
	page_t hpage;
	buf_read_frame(pid, header_page_num, &hpage);
//printf("\nget header에서 헤더정보: root_pnum: %ldfree_pnum:%ld\n\n", hpage.header_page.root_pnum, hpage.header_page.free_pnum);
	//printf("header found\n");
	return hpage;
}

pagenum_t find_leaf(int pid, uint64_t key){
	//printf("find leaf 실행\n");
	page_t hpage;
	int index = buf_read_frame(pid, header_page_num, &hpage);
	my_unlock(index);
	//pthread_mutex_unlock

	page_t ipage;
	int iindex = buf_read_frame(pid, hpage.header_page.root_pnum, &ipage);
	my_unlock(iindex);
	pagenum_t pagenum = hpage.header_page.root_pnum;
	if(pagenum == 0) return header_page_num;	
	int i = 0;

	while(!ipage.internal_page.is_leaf){
		i = 0;
		while(i< ipage.internal_page.num_of_k){
			if(key >= ipage.internal_page.keys[i].key) i++;
			else break;
		}
		if(i == 0) {
		pagenum = ipage.internal_page.l_pnum;
		index = buf_read_frame(pid, pagenum, &ipage);
		my_unlock(index);
		}		
		else{
		pagenum = ipage.internal_page.keys[i - 1].pnum;
		index = buf_read_frame(pid, pagenum, &ipage);
		my_unlock(index);
		}	
	}
	//printf("find leaf 끝,key: %ld가  pid: %d의 %ld페이지에 존재\n", key,pid,  pagenum);
	return pagenum;
}


pagenum_t trx_find_leaf(int pid, uint64_t key){
	//printf("trx_find leaf 실행\n");
	//헤더 읽기
	page_t hpage;
	int h_index = buf_read_frame(pid, header_page_num, &hpage);
	my_unlock(h_index);	//바로 unlock해주는게 맞는건가????

	//루트읽기
	page_t ipage;

	pagenum_t pnum = hpage.header_page.root_pnum;
	int r_index = buf_read_frame(pid, pnum, &ipage);
	my_unlock(r_index);	//바로 unlock해주는게 맞는건가????
	
	if(pnum == 0) return header_page_num;	
	int i = 0;
	while(!ipage.internal_page.is_leaf){
		i = 0;
		while(i< ipage.internal_page.num_of_k){
			if(key >= ipage.internal_page.keys[i].key) i++;
			else break;
		}
		if(i == 0) {
			pnum = ipage.internal_page.l_pnum;
			int index = buf_read_frame(pid, pnum, &ipage);
			my_unlock(index);
		}		
		else{
			pnum = ipage.internal_page.keys[i - 1].pnum;
			int index = buf_read_frame(pid, pnum, &ipage);
			my_unlock(index);
		}	
	}
	//printf("find leaf 끝,key가  pid: %d의 %ld페이지에 존재\n", pid, pnum);
	return pnum;
}





int update(int tid, int64_t key, char* value, int trx_id){
	int i = 0;
	//printf("trxid: %d, key: %ld를 update실행\n", trx_id, key);
	page_t hpage, page;
	int h_index = buf_read_frame(tid, header_page_num, &hpage);
	my_unlock(h_index);
	pagenum_t pagenum = trx_find_leaf(tid, key); //key가 존재할 만한 leaf 찾음
	//printf("여긴아니겠찌?\n");
	int index = buf_read_frame(tid, pagenum, &page);
	//my_unlock(index);
	if(pagenum == header_page_num){
	//printf("루트가없다\n");	 
	return -1;
	}	
	for(i = 0; i< page.leaf_page.num_of_k; i++)
	{	
		if((page.leaf_page.records[i].key) == key) break;
	}
	if(i == page.leaf_page.num_of_k){
		//printf("pid: %d 파일에서 %ld를 못찾음\n", pid, key);
		return -1;
	}
	//page의 i번째 index에 존재.
	//printf("진행\n");
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
	int ret = lock_acquire(tid, page.leaf_page.records[i].key, trx_id, 1, lock);
	//printf("lock 정보- trxid:%d\n", (lock)->trx_id);
	if(ret == AQUIRED){
		//printf("aquired\n");
		strcpy(page.leaf_page.records[i].value, value);
		buf_write_frame(tid, pagenum, &page);
		return 0;
	}
	else if(ret == NEED_TO_WAIT){
		//printf("need to wait\n");
		my_unlock(index);
		lock_wait(lock);
		int index2 = buf_read_frame(tid, pagenum, &page);
		strcpy(page.leaf_page.records[i].value, value);
		buf_write_frame(tid, pagenum, &page);
		my_unlock(index2);
		return 0;
	}
	else if(ret == DEADLOCK){
		my_unlock(index);
		my_abort(trx_id);
		return -1;
	}


	index = buf_read_frame(tid, pagenum, &page);
	/*val_t* val_str = next_val(trx_id);

	strcpy(val_str->val, page.leaf_page.records[i].value); //임시로저장
	val_str->pnum = pagenum;
	val_str->tid = tid;
	val_str->index = i;*/

	//printf("%s", page.leaf_page.records[i].value);

	strcpy(page.leaf_page.records[i].value, value); 
	buf_write_frame(tid, pagenum, &page);
	//my_unlock(index);
	//printf("i번째 key은 %ld이고, value는 %s 이다\n", page.leaf_page.records[i].key, page.leaf_page.records[i].value);
	//printf("에서 %s로 바뀌었다\n", page.leaf_page.records[i].value);
	//printf("find()끝\n");
	return 0;
}

val_t* next_val(int trx_id){
	trx_t* trx;
	HASH_FIND_INT(get_trx_table(), &trx_id, trx);
	val_t* val = trx->old_val;
	if(val == NULL){
		val = (val_t*)malloc(sizeof(val_t));
		return val;
	}
	while(val->next != NULL){
		val = val->next;
	}
	val->next = (val_t*)malloc(sizeof(val_t));
	
	return val->next;
}


int trx_find(int tid, uint64_t key, char* ret_val, int trx_id){
	//printf("trx:%d가 key: %ld의 find실행\n", trx_id, key);
	int i = 0;
	page_t hpage, page;
	int h_index = buf_read_frame(tid, header_page_num, &hpage);
	my_unlock(h_index);
	
	pagenum_t pagenum = trx_find_leaf(tid, key); //key가 존재할 만한 leaf 찾음
	
	int index = buf_read_frame(tid, pagenum, &page);
	
	
	if(pagenum == header_page_num){
	//printf("루트가없다\n");	 
	return -1;
	}	
	for(i = 0; i< page.leaf_page.num_of_k; i++)
	{	
		if((page.leaf_page.records[i].key) == key) break;
	}
	if(i == page.leaf_page.num_of_k){
		//printf("pid: %d 파일에서 %ld를 못찾음\n", tid, key);
		return -1;
	}
	//page의 i번째 index에 존재.
	lock_t* lock = (lock_t*)malloc(sizeof(lock_t));
	int ret = lock_acquire(tid, page.leaf_page.records[i].key, trx_id, 0, lock);
	if(ret == AQUIRED){
		strcpy(ret_val, page.leaf_page.records[i].value);
		my_unlock(index);
		return 0;
	}
	else if(ret == NEED_TO_WAIT){
		//printf("need to wait");
		my_unlock(index);
		lock_wait(lock);
		int index2 = buf_read_frame(tid, pagenum, &page);
		strcpy(ret_val, page.leaf_page.records[i].value);
		my_unlock(index2);
		return 0;
	}
	else if(ret == DEADLOCK){
		my_unlock(index);
		my_abort(trx_id);
		return -1;
	}
	//printf("findㅈ진행\n");
	index = buf_read_frame(tid, pagenum, &page);
	my_unlock(index);
	strcpy(ret_val, page.leaf_page.records[i].value);
	//printf("i번째 key은 %ld이고, value는 %s 이다\n", page.leaf_page.records[i].key, page.leaf_page.records[i].value);
	//printf("trx_id: %d가 찾기를 key: %ld가 %ld번째 페이지에있다\n", trx_id, key, pagenum);
	//printf("find()끝\n");
	return 0;
}




int find(int pid, uint64_t key, char* ret_val){
	//printf("find()실행\n");
	int i = 0;
	page_t hpage;
	int index = buf_read_frame(pid, header_page_num, &hpage);
	my_unlock(index);
	pagenum_t pagenum = find_leaf(pid, key);
	page_t page;
	//printf("그냥있는거\n");
	int index2 = buf_read_frame(pid, pagenum, &page);
	my_unlock(index2);
	
	if(pagenum == header_page_num){
	//printf("루트가없다\n");	 
	return -1;
	}	
	for(i = 0; i< page.leaf_page.num_of_k; i++)
	{	
		if((page.leaf_page.records[i].key) == key) break;
	}
	if(i == page.leaf_page.num_of_k){
		//printf("pid: %d 파일에서 %ld를 못찾음\n", pid, key);
		return -1;
	}
	strcpy(ret_val, page.leaf_page.records[i].value);
	//printf("i번째 key은 %ld이고, value는 %s 이다\n", page.leaf_page.records[i].key, page.leaf_page.records[i].value);
	//printf("pid: %d 의 key: %ld가 %ld번째 페이지에있다\n", pid, key, pagenum);
	//printf("find()끝\n");
	return 0;
}

/*page_t find_leaf(uint64_t key, pagenum_t* ret_num) {
	printf("%ld 가 어떤 leaf에 있을까?\n", key);
	uint32_t i = 0;
	page_t hpage = get_header_page();
	page_t ipage = get_root_page();
	int quit = 1;
	pagenum_t next = hpage.header_page.root_pnum;
		
	while(ipage.internal_page.is_leaf == 0 || quit)
	{	
		if(key < ipage.internal_page.keys[i].key) next = ipage.internal_page.l_pnum; 
		else
		{
			while (i < ipage.internal_page.num_of_k) 
			{
				if(key > ipage.internal_page.keys[i].key) i++;
				else if(key = ipage.internal_page.keys[i].key) {
				quit = 0;
				break;		
				}
				else break;
			}
		}
		next = ipage.internal_page.keys[i].pnum;
		file_read_page(next, &ipage);
		i = 0;
	}
	*ret_num = next;
	file_read_page(next, &ipage);
	printf("pagenum:%ld 이쯤에 있고 페이지정보는 num of k: %d\n", next, ipage.leaf_page.num_of_k);
	

	return ipage;
}
*/


int cut( int length ) {
    if (length % 2 == 0)
        return length/2;
    else
        return length/2 + 1;
}

/* Creates a new general node, which can be adapted
 * to serve as either a leaf or an internal node.
 */
page_t make_internal_page(void){
	page_t page;
	page.internal_page.is_leaf = 0;
	page.internal_page.num_of_k = 0;
	return page;
}

page_t make_leaf_page(void){
	page_t page = make_internal_page();
	page.leaf_page.is_leaf = 1;
	return page;
}

/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(page_t ppage, pagenum_t l_pnum)
{
	int left_index = 0;
	//printf("parent의 numofkey:%d\n", ppage.internal_page.num_of_k);
	while(left_index < ppage.internal_page.num_of_k && ppage.internal_page.keys[left_index].pnum != l_pnum){
	//printf("현재 left index:%d 번째에 parent가 가르키는곳:%ld\n", left_index, ppage.internal_page.keys[left_index].pnum);
	left_index++;
	}
	return left_index;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_leaf(int pid, pagenum_t pagenum, uint64_t key, char* value ) { //leaf에 record삽입
	//printf("leaf아주 널널하니 그냥넣어봄\n");
	page_t lpage;
	buf_read_frame(pid, pagenum, &lpage);	
	int i, insertion_point;
	insertion_point = 0;
	while(insertion_point < lpage.leaf_page.num_of_k && lpage.leaf_page.records[insertion_point].key < key) insertion_point++;
	for(i = lpage.leaf_page.num_of_k; i >insertion_point; i--){
		lpage.leaf_page.records[i] = lpage.leaf_page.records[i - 1];
	}
	lpage.leaf_page.records[insertion_point].key = key;
	strcpy(lpage.leaf_page.records[insertion_point].value, value);
	lpage.leaf_page.num_of_k++;
	buf_write_frame(pid, pagenum, &lpage);
	//printf("%ld를 썼다!\n",lpage.leaf_page.records[insertion_point].key);
	

	return insertion_point;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(int pid, pagenum_t pagenum, uint64_t key, char* value) {
	//printf("leaf분열후 인서트진행\n");
	record temp[leaf_order];
	page_t lpage;
	buf_read_frame(pid, pagenum, &lpage);
	page_t npage = make_leaf_page();
	pagenum_t new_num = buf_alloc_page(pid);
	
	int i, j, split;
	uint64_t new_key;

	int insertion_index = 0;
	while(insertion_index <leaf_order - 1 && lpage.leaf_page.records[insertion_index].key < key) insertion_index++;

	for(i = 0, j = 0; i <lpage.leaf_page.num_of_k; i++, j++){
		if( j == insertion_index) j++;
		temp[j] = lpage.leaf_page.records[i];
	}
	temp[insertion_index].key = key;
	strcpy(temp[insertion_index].value, value);
	lpage.leaf_page.num_of_k = 0;

	split = cut(leaf_order - 1);
	
	for(i = 0; i < split; i++){
	lpage.leaf_page.records[i] = temp[i];
	lpage.leaf_page.num_of_k++;	
	}
	for(i = split, j = 0; i < leaf_order; i++, j++){
	npage.leaf_page.records[j] = temp[i];
	//printf("temp key:%ld ", temp[i].key);
	npage.leaf_page.num_of_k++;
	}

	npage.leaf_page.r_pnum = lpage.leaf_page.r_pnum;
	lpage.leaf_page.r_pnum = new_num;

	npage.leaf_page.parent_pnum = lpage.leaf_page.parent_pnum;
	new_key = npage.leaf_page.records[0].key;
	buf_write_frame(pid, pagenum, &lpage);
	//printf("과연 실행될까11?\n");
	buf_write_frame(pid, new_num, &npage);
	//printf("과연 실행될까22?\n");
	insert_into_parent(pid, pagenum, new_num, new_key);
	//printf("leaf 분열끝\n");
}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(int pid, pagenum_t p_pagenum, int right_index, uint64_t key, pagenum_t r_pnum) {
	//printf("insert_into_node\n");  
	page_t ppage;
	buf_read_frame(pid,p_pagenum, &ppage);
	int i;
    for (i = ppage.internal_page.num_of_k - 1; i >= right_index; i--) {
        ppage.internal_page.keys[i+1] = ppage.internal_page.keys[i];
    }
	ppage.internal_page.keys[right_index].pnum = r_pnum;
	ppage.internal_page.keys[right_index].key = key;
	ppage.internal_page.num_of_k++;
	buf_write_frame(pid, p_pagenum, &ppage);
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(int pid, pagenum_t p_pnum, int right_index, uint64_t key, pagenum_t r_pnum) {
	//printf("\ninsert into node after_splitting\n");
	int i, j, split;
	page_t ppage, rpage;
	buf_read_frame(pid, p_pnum, &ppage);
	key_pnum temp[internal_order];
	//printf("right index: %d\n", right_index);
	for(i = 0, j = 0; i< ppage.internal_page.num_of_k; i++, j++)
	{
		if(j == right_index) j++;
		temp[j] = ppage.internal_page.keys[i];
	}
	temp[right_index].key = key;
	temp[right_index].pnum = r_pnum;
	for(int k = 0; k<= ppage.internal_page.num_of_k; k++)
{
	//printf("temp의 %d번째 들어있는수: %ld\n",k, temp[k].key);

}
	 split = cut(internal_order);

	page_t npage = make_internal_page();
	pagenum_t n_pnum = buf_alloc_page(pid);
	//printf("새로운 pnum: %ld\n", n_pnum);

	ppage.internal_page.num_of_k = 0;
	for( i = 0; i < split; i++){
		ppage.internal_page.keys[i] = temp[i];
		ppage.internal_page.num_of_k++;
	}
	uint64_t k_prime = temp[split].key;

	npage.internal_page.l_pnum = temp[split].pnum;

	for(++i, j = 0; i < internal_order; i++, j++){
		npage.internal_page.keys[j] = temp[i];
		npage.internal_page.num_of_k++;
	}
	
	npage.internal_page.parent_pnum = ppage.internal_page.parent_pnum;
	//printf("ppage의 parent pnum: %ld\n", ppage.internal_page.parent_pnum);
	page_t cpage;
	buf_read_frame(pid, npage.internal_page.l_pnum, &cpage);
	cpage.internal_page.parent_pnum = n_pnum;
	buf_write_frame(pid, npage.internal_page.l_pnum, &cpage);
	for(i = 0; i < npage.internal_page.num_of_k; i++){
		buf_read_frame(pid, npage.internal_page.keys[i].pnum, &cpage);
		cpage.internal_page.parent_pnum = n_pnum;
		buf_write_frame(pid, npage.internal_page.keys[i].pnum, &cpage);
	}
	buf_write_frame(pid, n_pnum, &npage);
	buf_write_frame(pid, p_pnum, &ppage);
	insert_into_parent(pid, p_pnum, n_pnum, k_prime);
	
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int insert_into_parent(int pid, pagenum_t lnum, pagenum_t rnum, uint64_t new_key) {
	//printf("insert into parent\n");
    int left_index, right_index;
	page_t lpage, rpage;
	int index = buf_read_frame(pid, lnum, &lpage);
	my_unlock(index);
	//printf("lnum: %ld, rnum: %ld\n", lnum, rnum);
	page_t ppage;
	pagenum_t p_pnum = lpage.internal_page.parent_pnum;
	
    if (p_pnum == 0){
		//printf("새로운 root\n");
        insert_into_new_root(pid, lnum, rnum, new_key);
	return 0;
	}
	
	index = buf_read_frame(pid, p_pnum, &ppage);
	my_unlock(index);
 	left_index = get_left_index(ppage, lnum);
	right_index = left_index + 1;
	//printf("left index:%d right index:%d\n", left_index, right_index);

    /* Simple case: the new key fits into the node. 
     */

    if (ppage.internal_page.num_of_k < internal_order - 1)
        insert_into_node(pid, p_pnum, right_index, new_key, rnum);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    else insert_into_node_after_splitting(pid, p_pnum, right_index, new_key, rnum);

	return 0;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(int pid, pagenum_t l_pagenum, pagenum_t r_pagenum, uint64_t key) {
	page_t lpage, rpage;
	//printf("insert into NEW ROOT\n");	
	buf_read_frame(pid, l_pagenum, &lpage);
	buf_read_frame(pid, r_pagenum, &rpage);

	//printf("%ld 와 %ld를 삽입\n", l_pagenum, r_pagenum);
	pagenum_t root_pnum = buf_alloc_page(pid);
	//printf("새로운 root pnum: %ld\n", root_pnum);
	page_t root_page = make_internal_page();
	root_page.internal_page.keys[0].key = key;
	root_page.internal_page.l_pnum = l_pagenum;
	root_page.internal_page.keys[0].pnum = r_pagenum; 
	root_page.internal_page.num_of_k++;
	root_page.internal_page.parent_pnum = 0;

	lpage.internal_page.parent_pnum = root_pnum;
	rpage.internal_page.parent_pnum = root_pnum;
	buf_write_frame(pid, root_pnum, &root_page);
	buf_write_frame(pid, l_pagenum, &lpage);
	buf_write_frame(pid, r_pagenum, &rpage);

	page_t hpage = get_header_page(pid);
	hpage.header_page.root_pnum = root_pnum;
	buf_write_frame(pid, header_page_num, &hpage);
}



/* First insertion:
 * start a new tree.
 */
void start_new_tree(int pid, uint64_t key, char* value) {
	//printf("\n새로한번 시작해볼까?\n");
	page_t rpage;
	pagenum_t r_pnum = buf_alloc_page(pid);
	page_t hpage = get_header_page(pid);
	hpage.header_page.root_pnum = r_pnum;
	rpage.leaf_page.parent_pnum = 0;
	rpage.leaf_page.is_leaf = 1;
	rpage.leaf_page.num_of_k = 1;
	rpage.leaf_page.r_pnum = 0;
	rpage.leaf_page.records[0].key = key;
	strcpy(rpage.leaf_page.records[0].value, value);
	buf_write_frame(pid, r_pnum, &rpage);
	buf_write_frame(pid, header_page_num, &hpage);
	//printf("%ld페이지 0번째에 %ld 넣기 성공\n\n", r_pnum, key);
	//printf("루트정보: numofk: %d\n",rpage.internal_page.num_of_k );
}




/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int insert(int pid, uint64_t key, char* value ) {
	//printf("\ninsert %ld 실행\n", key);
	page_t hpage;
//	printf("과연?\n");
	int index = buf_read_frame(pid, header_page_num, &hpage);
	my_unlock(index);
	pagenum_t pagenum;
	

    page_t lpage = make_leaf_page();
	char temp;			//나중에오류생길수도있음 동적할당??
    /* The current implementation ignores
     * duplicates.
     */
	if (hpage.header_page.root_pnum == 0) {
		//printf("root 새로 생성\n");
		start_new_tree(pid, key, value);
		//printf("root 새로 생성\n");
		return 0;
	}
	

    if (find(pid, key, &temp) != -1){
		//printf("이미있음\n");
		return -1;
	}  //insertion fail
	//printf("here\n");	
    /* Create a new record for the
     * value.
     */

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */



    /* Case: the tree already exists.
     * (Rest of function body.)
     */
	
    pagenum = find_leaf(pid, key);
	//printf("일단찾음\n");
	int lindex = buf_read_frame(pid, pagenum, &lpage);
	my_unlock(lindex);
    /* Case: leaf has room for key and pointer.
     */
    if (lpage.leaf_page.num_of_k < leaf_order - 1) {
	//printf("%d < %d\n", lpage.leaf_page.num_of_k, leaf_order- 1);
	
    //printf("insert into leaf실행\n");
	insert_into_leaf(pid, pagenum, key, value);	
    return 0;
	}


    /* Case:  leaf must be split.
     */
	//printf("insert_into_leaf_after_splitting\n");
    insert_into_leaf_after_splitting(pid, pagenum, key, value);
	//printf("insert 끝\n");
	return 0;
}





//deletion//

void remove_entry_from_page(int pid, pagenum_t pagenum, int key){
	//printf("\nremove entry 실행\n");
	page_t lpage;

	buf_read_frame(pid, pagenum, &lpage);
	int i, deletion_point;
	deletion_point = 0;
	//printf("page의 key개수%d\n", lpage.leaf_page.num_of_k);
	//printf("%ld의 페이지를 본다\n", pagenum);
	if(lpage.leaf_page.is_leaf == 1)
	{
		while(lpage.leaf_page.records[deletion_point].key != key)
		{
			//printf("leaf의 %d 번째에는 %ld가 있네?",deletion_point, lpage.leaf_page.records[deletion_point].key);
			deletion_point++;
		}
		for(i = deletion_point+1; i <lpage.leaf_page.num_of_k; i++)
		{
		
			lpage.leaf_page.records[i - 1] = lpage.leaf_page.records[i];
		}	
	}
	else
	{
		while(lpage.internal_page.keys[deletion_point].key != key)
		{
			//printf("internal의 %d 번째에는 %ld가 있네?",deletion_point, lpage.leaf_page.records[deletion_point].key);
			deletion_point++;
		}
		for(i = deletion_point+1; i <lpage.internal_page.num_of_k; i++)
		{
		//printf("%ld->", lpage.internal_page.keys[i-1].key);
		lpage.internal_page.keys[i - 1] = lpage.internal_page.keys[i];
		//printf("%ld\n", lpage.internal_page.keys[i-1].key);
		}	
	}
	//printf("deletion point:%d", deletion_point);
	lpage.leaf_page.num_of_k--;
	buf_write_frame(pid, pagenum, &lpage);
}


int adjust_root(int pid)
{
	//printf("adjust root 실행\n");
	page_t npage;
	page_t hpage = get_header_page(pid);
	page_t rpage;
	int index = buf_read_frame(pid, hpage.header_page.root_pnum, &rpage);
	my_unlock(index);
	pagenum_t n_pnum;
	if(rpage.internal_page.num_of_k > 0) return 0;
	if(rpage.internal_page.is_leaf ==0)
	{
		n_pnum = rpage.internal_page.l_pnum;
		buf_read_frame(pid, n_pnum, &npage);	//root의 leftmost child 가져옴
		npage.internal_page.parent_pnum = 0; //leftmost child를 root로 바꿈
		//printf("npnum: %ld\n", n_pnum);
		buf_free_page(pid, hpage.header_page.root_pnum);		
		hpage.header_page.root_pnum = n_pnum;
		buf_write_frame(pid, n_pnum, &npage);
		buf_write_frame(pid, header_page_num, &hpage);		
		
	}
	else{ //root가 사라짐
	pagenum_t temp_pnum = hpage.header_page.root_pnum;	
	hpage.header_page.root_pnum = 0;
	buf_write_frame(pid, header_page_num, &hpage);
	buf_free_page(pid, temp_pnum);
	}
	return 0;
}


int coalesce_pages(int pid, int k_index, int c_index, pagenum_t c_pnum)
{	
	//printf("coal뭐시기 실행\n");
	page_t cpage, ppage, npage;
	
	int cindex = buf_read_frame(pid, c_pnum, &cpage);

	pagenum_t p_pnum = cpage.leaf_page.parent_pnum;
	pagenum_t n_pnum;
	
	int pindex = buf_read_frame(pid, p_pnum, &ppage);

	uint64_t p_key = ppage.internal_page.keys[k_index].key;

	//printf("p_key: %ld  neighbor_index:%d \n", p_key, c_index - 1);	

	if(cpage.leaf_page.is_leaf)
	{
		//printf("leaf 합쳐짐");
		if(c_index == -1)
		{	
			my_unlock(cindex);
			
			ppage.internal_page.l_pnum = ppage.internal_page.keys[0].pnum;
			//printf("num of k:%d\n", ppage.internal_page.num_of_k);
			buf_write_frame(pid, p_pnum, &ppage);
			
		}
		buf_free_page(pid, c_pnum);
			
	}
	else if(!cpage.internal_page.is_leaf)
	{
		//cpage의 옆 페이지인 check page가 full인지 아닌지 확인
		pagenum_t check_pnum;
		page_t check_page;
		if(c_index == -1) check_pnum = ppage.internal_page.keys[0].pnum;
		else if(c_index == 0) check_pnum = ppage.internal_page.l_pnum;
		else check_pnum = ppage.internal_page.keys[c_index - 1].pnum;

		int index = buf_read_frame(pid, check_pnum, &check_page);
		my_unlock(index);
		if(check_page.internal_page.num_of_k == internal_order - 1){ //check page가 full이면
			my_unlock(cindex);
			my_unlock(pindex);
			redistribute_pages(pid, p_pnum, c_index);
			return 0;
		}
		else{ //full이 아니면 그 옆page에 합쳐짐
			//printf("internal이 합쳐짐\n");
			my_unlock(pindex);
			if(c_index == -1)	//parent의 p_key와 npage의 모든 keys와 pnum을 cpage로 이동
			{	
				//printf("왼쪽페이지에 합쳐짐\n"); //npage의 모든 keys가 npage로 이동
				n_pnum = ppage.internal_page.keys[0].pnum;
				int nindex = buf_read_frame(pid, n_pnum, &npage);
				my_unlock(nindex);
				cpage.internal_page.keys[0].key = p_key;
				cpage.internal_page.keys[0].pnum = npage.internal_page.l_pnum;
				page_t temp;
				buf_read_frame(pid, cpage.internal_page.keys[0].pnum, &temp);
				temp.internal_page.parent_pnum = c_pnum;
				buf_write_frame(pid, cpage.internal_page.keys[0].pnum, &temp);
		
				cpage.internal_page.num_of_k++;
				for(int i = 0; i< npage.leaf_page.num_of_k; i++)
				{
					cpage.internal_page.keys[i + 1] = npage.internal_page.keys[i];
					cpage.internal_page.num_of_k++;
				}
				buf_write_frame(pid, c_pnum, &cpage);
		
				for(int i = 0; i < cpage.internal_page.num_of_k; i++){
				page_t temp;
				buf_read_frame(pid, cpage.internal_page.keys[i].pnum, &temp);
				temp.internal_page.parent_pnum = c_pnum;
				buf_write_frame(pid, cpage.internal_page.keys[i].pnum, &temp);
			
				}
	
				buf_free_page(pid, n_pnum);
			}
			else	//npage에 합쳐짐->parent의 p_key와 cpage의 유일한 child만 옮기면 됨.
			{
				//printf("npage에 합쳐짐->parent의 p_key와 cpage의 유일한 child만 옮기면 됨.\n");
				//printf("c_index: %d\n", c_index);
				if(c_index == 0) n_pnum = ppage.internal_page.l_pnum;
				else n_pnum = ppage.internal_page.keys[c_index - 1].pnum;
				int pindex = buf_read_frame(pid, n_pnum, &npage);
				npage.internal_page.keys[npage.internal_page.num_of_k].key = p_key;
				npage.internal_page.keys[npage.internal_page.num_of_k].pnum = cpage.internal_page.l_pnum;
				page_t temp;
				buf_read_frame(pid, npage.internal_page.keys[npage.internal_page.num_of_k].pnum, &temp);
				temp.internal_page.parent_pnum = n_pnum;
				buf_write_frame(pid, npage.internal_page.keys[npage.internal_page.num_of_k].pnum, &temp);	
				
				npage.internal_page.num_of_k++;
				
				buf_write_frame(pid, n_pnum, &npage);
				my_unlock(pindex);
				buf_free_page(pid, c_pnum);
			}
		}
		//합쳐야할 npage가 full일떄 구현->redis
	}
	delete_entry(pid, p_pnum, p_key);
	return 0;
}

int redistribute_pages(int pid, pagenum_t p_pnum, int c_index){
	//printf("redis실행\n");
	page_t ppage, cpage, npage;

	buf_read_frame(pid, p_pnum, &ppage);
 
	if(c_index == -1){
		//printf("c_index: %d\n", c_index);
		buf_read_frame(pid, ppage.internal_page.keys[0].pnum, &npage);
		buf_read_frame(pid, ppage.internal_page.l_pnum, &cpage); 
		cpage.internal_page.keys[0].key = ppage.internal_page.keys[0].key;
		cpage.internal_page.keys[0].pnum = npage.internal_page.l_pnum;
		cpage.internal_page.num_of_k++;
		
		ppage.internal_page.keys[0].key = npage.internal_page.keys[0].key;
		npage.internal_page.l_pnum = npage.internal_page.keys[0].pnum;
		//printf("npage의 가장 왼쪽 key: %ld\n", npage.internal_page.keys[0].key);
		buf_write_frame(pid, ppage.internal_page.keys[0].pnum, &npage);
		remove_entry_from_page(pid, ppage.internal_page.keys[0].pnum, npage.internal_page.keys[0].key);
		buf_read_frame(pid, ppage.internal_page.keys[0].pnum, &npage);
		buf_write_frame(pid, ppage.internal_page.keys[0].pnum, &npage);	
		buf_write_frame(pid, ppage.internal_page.l_pnum, &cpage);
		buf_write_frame(pid, p_pnum, &ppage);	

		page_t temp;
		buf_read_frame(pid, cpage.internal_page.keys[0].pnum, &temp);
		temp.internal_page.parent_pnum = ppage.internal_page.l_pnum;
		buf_write_frame(pid, cpage.internal_page.keys[0].pnum, &temp);

		return 0; 
	}
	else{
		pagenum_t n_pnum, c_pnum;
		if(c_index == 0) n_pnum = ppage.internal_page.l_pnum;
		else n_pnum = ppage.internal_page.keys[c_index - 1].pnum;
		c_pnum = ppage.internal_page.keys[c_index].pnum;
		
		buf_read_frame(pid, n_pnum, &npage);
		buf_read_frame(pid, c_pnum, &cpage);
		
		cpage.internal_page.keys[0].key = ppage.internal_page.keys[c_index].key;
		cpage.internal_page.keys[0].pnum = cpage.internal_page.l_pnum;
		cpage.internal_page.num_of_k++;
		//printf("cpage의 lpnum: %ld\n, cpnum:%ld", cpage.internal_page.l_pnum, c_pnum);
		//printf("npage의 numofk: %d\n", npage.internal_page.num_of_k);
		cpage.internal_page.l_pnum = npage.internal_page.keys[npage.internal_page.num_of_k-1].pnum;
		//printf("npage의 가장 오른쪽pnum: %ld\n", npage.internal_page.keys[npage.internal_page.num_of_k-1].pnum);
		page_t temp;
		buf_read_frame(pid, cpage.internal_page.l_pnum, &temp);
		temp.internal_page.parent_pnum = c_pnum;
		buf_write_frame(pid, cpage.internal_page.l_pnum, &temp);

		ppage.internal_page.keys[c_index].key = npage.internal_page.keys[npage.internal_page.num_of_k-1].key;
		npage.internal_page.num_of_k--;

		buf_write_frame(pid, p_pnum, &ppage);
		buf_write_frame(pid, c_pnum, &cpage);
		buf_write_frame(pid, n_pnum, &npage);
		return 0;
	}

}



int get_neighbor_index(int pid, pagenum_t pagenum){
	//printf("get neigh실행\n");
	page_t cpage, ppage;
	int i = 0;
	int cindex = buf_read_frame(pid, pagenum, &cpage);
	my_unlock(cindex);
	
	pagenum_t p_pnum = cpage.leaf_page.parent_pnum;
	int pindex = buf_read_frame(pid, p_pnum, &ppage);
	my_unlock(pindex);
	//printf("\n\n\n\n어디냐\n\n\n\n");
	if(ppage.internal_page.l_pnum == pagenum) {
	//printf("neigh: -2\n");
	return -2;
	}
	while(i< ppage.internal_page.num_of_k){
		if(ppage.internal_page.keys[i].pnum == pagenum){
			//printf("neigh: %d\n", i - 1);			 
			return i - 1;
		}
		i++;
	}

	//printf("no pagenum found in parent page\n");
	//printf("parent pagenum: %ld\n", p_pnum);
}

int delete_entry(int pid, pagenum_t pagenum, uint64_t key)
{
	char abc;
	char* pabc = &abc;
	page_t page;
	page_t hpage;
	int index = buf_read_frame(pid, header_page_num, &hpage);
	my_unlock(index);
	remove_entry_from_page(pid, pagenum, key);
	int pindex = buf_read_frame(pid, pagenum, &page);
	my_unlock(pindex);
	//printf("지우고자하는 곳의 num_of_k: %d\n", page.internal_page.num_of_k);
	if(hpage.header_page.root_pnum == pagenum)
	{
		adjust_root(pid);	
		return 0;
	}
	int neighbor_index, k_index;
	pagenum_t neighbor_pnum;
	
	neighbor_index = get_neighbor_index(pid, pagenum);
	//printf("요기?\n");
	if(neighbor_index == -2) k_index = 0;
	else k_index = neighbor_index + 1;

	if(page.internal_page.num_of_k == 0)
	{
		coalesce_pages(pid, k_index,neighbor_index + 1, pagenum);
	}
	return 0;
}





int delete(int pid, uint64_t key) {
	int a;
	char temp;
	char* ptemp = &temp;

	if(find(pid, key, ptemp) == -1){
		printf("존재하지않는 키 입력\n");
		return -1;
	}  
	pagenum_t pnum = find_leaf(pid, key);
	delete_entry(pid, pnum, key);
	printf("delete 종료\n");
	return 0;
}






