
#define Version "1.14"


#include "bpt.h"
#include "file.h"
#include "db.h"



void open_data(char* pathname)
{
	page_t hpage;
	int start_new = open_file(pathname, &hpage);
	if(start_new)
	{
	printf("start new\n");
	hpage.header_page.free_pnum = 0;
	hpage.header_page.root_pnum = 0;
	hpage.header_page.num_of_p = 1U;
	file_write_page(header_page_num, &hpage);

	page_t temp;
	file_read_page(0, &temp);
	//printf("제발 1: %ld\n", temp.header_page.num_of_p);
	}
}


page_t get_next_page(int i, page_t ipage)
{
	page_t new_page;
	pagenum_t pnum = ipage.internal_page.keys[i].pnum;
	file_read_page(pnum, &new_page);
	return new_page;
}
//insert에서 이용
//ipage는 무조건 internal page
/*
char* index_find(uint64_t key) 
{
	uint32_t i = 0;
	pagenum_t next;
	page_t hpage, ipage;
	file_read_page(header_page_num, &hpage);
	pagenum_t root_page_num = hpage.header_page.root_pnum;
	file_read_page(root_page_num, &ipage);	
	while(ipage.internal_node.is_leaf == 0)
	{	
		if(key < ipage.internal_page.keys[i].key) next = ipage.internal_page.l_pnum; 
		else{
			while (i < page.internal_page.num_of_k) {
			if(key > ipage.internal_page.keys[i].key) i++;
			else break;
			}
		}
		next = ipage.internal_page.keys[i].pnum;
		file_read_page(next, &ipage);
		i = 0;
	}
	page_t lpage = ipage;
	while( i < lpage.leaf_page.num_of_k){
		if (key > lpage.leaf_page.records[i].key) i++
		else if(key == lpage.leaf_page.records[i].key){
			 return lpage.leaf_page.records[i].value;
			return 0;
		}
	}
}*/


page_t get_header_page(){
	page_t hpage;
	file_read_page(header_page_num, &hpage);
//printf("\nget header에서 헤더정보: root_pnum: %ldfree_pnum:%ld\n\n", hpage.header_page.root_pnum, hpage.header_page.free_pnum);
	return hpage;
}


page_t get_root_page(){
	page_t hpage;
	page_t rpage;
	file_read_page(header_page_num, &hpage);
	pagenum_t root_pnum = hpage.header_page.root_pnum;
	file_read_page(root_pnum, &rpage);
	return rpage;
}

page_t find_leaf(uint64_t key, pagenum_t* ret_num) {
	
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

	return ipage;
}



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
	page.internal_page.is_leaf = 1;
	page.internal_page.num_of_k = 0;
	return page;
}

page_t make_leaf_page(void){
	page_t page = make_internal_page();
	page.leaf_page.is_leaf = 0;
	return page;
}

/* Helper function used in insert_into_parent
 * to find the index of the parent's pointer to 
 * the node to the left of the key to be inserted.
 */
int get_left_index(page_t ppage, pagenum_t l_pnum)
{
	int left_index = 0;
	while(left_index <= ppage.internal_page.num_of_k && ppage.internal_page.keys[left_index].pnum != l_pnum) left_index++;
	return left_index;
}


/* Inserts a new pointer to a record and its corresponding
 * key into a leaf.
 * Returns the altered leaf.
 */
int insert_into_leaf(pagenum_t pagenum, page_t l_page, uint64_t key, char* value ) { //leaf에 record삽입
	page_t lpage;

	file_read_page(pagenum, &lpage);	//pagenum의 페이지를 lpage로 읽는다
	int i, insertion_point;
	insertion_point = 0;
	while(insertion_point < lpage.leaf_page.num_of_k && lpage.leaf_page.records[insertion_point].key < key) insertion_point++;
	for(i = lpage.leaf_page.num_of_k; i >insertion_point; i--){
	
		lpage.leaf_page.records[i] = lpage.leaf_page.records[i - 1];
	}
	lpage.leaf_page.records[insertion_point].key = key;
	strcpy(lpage.leaf_page.records[insertion_point].value, value);
	//printf("num_of_k: %d\n", lpage.leaf_page.num_of_k);
	lpage.leaf_page.num_of_k++;
	file_write_page(pagenum, &lpage);
//	printf("%ld를 썼다!\n",lpage.leaf_page.records[insertion_point].key);
	

//	printf("%ld번째 페이지 %d번째 레코드에 %ld가 써짐\n", pagenum, insertion_point, tpage.leaf_page.records[insertion_point].key);
	//printf("num_of_k: %d\n", tpage.leaf_page.num_of_k);
//	if(tpage.leaf_page.records[insertion_point].key == 0) insert_into_leaf(pagenum, lpage, key, value);
	return insertion_point;
}


/* Inserts a new key and pointer
 * to a new record into a leaf so as to exceed
 * the tree's order, causing the leaf to be split
 * in half.
 */
void insert_into_leaf_after_splitting(pagenum_t pagenum, page_t lpage, uint64_t key, char* value) {
	
	record temp[leaf_order];
	page_t npage = make_leaf_page();
	pagenum_t new_num = file_alloc_page();
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
	npage.leaf_page.num_of_k++;
	}
	npage.leaf_page.r_pnum = lpage.leaf_page.r_pnum;
	lpage.leaf_page.r_pnum = new_num;

	npage.leaf_page.parent_pnum = lpage.leaf_page.parent_pnum;
	new_key = npage.leaf_page.records[0].key;
	insert_into_parent(pagenum, new_num, lpage, new_key, npage);
	file_write_page(new_num, &npage);

}


/* Inserts a new key and pointer to a node
 * into a node into which these can fit
 * without violating the B+ tree properties.
 */
void insert_into_node(pagenum_t p_pagenum, page_t ppage, int left_index, uint64_t key, pagenum_t r_pagenum) {
	printf("insert_into_node\n");    
	int i;
    for (i = ppage.internal_page.num_of_k; i > left_index; i--) {
        ppage.internal_page.keys[i+1] = ppage.internal_page.keys[i];
    }
	ppage.internal_page.keys[left_index + 1].pnum = r_pagenum;
	ppage.internal_page.keys[left_index + 1].key = key;
	ppage.internal_page.num_of_k++;
	file_write_page(p_pagenum, &ppage);
}


/* Inserts a new key and pointer to a node
 * into a node, causing the node's size to exceed
 * the order, and causing the node to split into two.
 */
void insert_into_node_after_splitting(pagenum_t p_pnum, page_t ppage, int left_index, uint64_t key, pagenum_t r_pnum) {
	printf("insert into node after_splitting\n");
	int i, j, split;
	key_pnum temp[internal_order];
	for(i = 0, j = 0; i< ppage.internal_page.num_of_k; i++, j++)
	{
		if(j == left_index + 1) j++;
		temp[j] = ppage.internal_page.keys[i];
	}
	temp[left_index].key = key;
	temp[left_index].pnum = r_pnum;
	
	 split = cut(internal_order);

	page_t npage = make_internal_page();
	pagenum_t n_pnum = file_alloc_page();
	

	ppage.internal_page.num_of_k = 0;
	for( i = 0; i < split - 1; i++){
		ppage.internal_page.keys[i] = temp[i];
		ppage.internal_page.num_of_k++;
	}
	uint64_t k_prime = temp[split - 1].key;
	npage.internal_page.l_pnum = temp[split - 1].pnum;
	for(++i, j = 0; i < internal_order; i++, j++){
		npage.internal_page.keys[j] = temp[i];
		npage.internal_page.num_of_k++;
	}
	npage.internal_page.parent_pnum = ppage.internal_page.parent_pnum;
	page_t cpage;
	file_read_page(npage.internal_page.l_pnum, &cpage);
	cpage.internal_page.parent_pnum = n_pnum;
	file_write_page(npage.internal_page.l_pnum, &cpage);
	for(i = 0; i < npage.internal_page.num_of_k; i++){
		file_read_page(npage.internal_page.keys[i].pnum, &cpage);
		cpage.internal_page.parent_pnum = n_pnum;
		file_write_page(npage.internal_page.keys[i].pnum, &cpage);
	}
    insert_into_parent(p_pnum, n_pnum, ppage, k_prime, npage);
	
}



/* Inserts a new node (leaf or internal node) into the B+ tree.
 * Returns the root of the tree after insertion.
 */
int insert_into_parent(pagenum_t lnum, pagenum_t rnum, page_t lpage, uint64_t new_key, page_t rpage) {
	printf("insert into parent\n");
    int left_index;

	page_t ppage = make_internal_page();
   
	pagenum_t p_pagenum = lpage.internal_page.parent_pnum;
 

    if (p_pagenum == 0){
        insert_into_new_root(lnum, rnum, lpage, new_key, rpage);
	return 0;
	}
	
    left_index = get_left_index(ppage, lnum);


    /* Simple case: the new key fits into the node. 
     */

    if (ppage.internal_page.num_of_k < internal_order - 1)
        insert_into_node(p_pagenum, ppage, left_index, new_key, rnum);

    /* Harder case:  split a node in order 
     * to preserve the B+ tree properties.
     */

    else insert_into_node_after_splitting(p_pagenum, ppage, left_index, new_key, rnum);

	return 0;
}


/* Creates a new root for two subtrees
 * and inserts the appropriate key into
 * the new root.
 */
void insert_into_new_root(pagenum_t l_pagenum, pagenum_t r_pagenum, page_t lpage, uint64_t key, page_t rpage) {
	printf("insert into NEW ROOT\n");
    //node * root = make_node();
	pagenum_t root_pnum = file_alloc_page();
	page_t root_page = make_internal_page();
	root_page.internal_page.keys[0].key = key;
	root_page.internal_page.keys[0].pnum = l_pagenum;
	root_page.internal_page.keys[1].pnum = r_pagenum; 
	root_page.internal_page.num_of_k++;
	root_page.internal_page.parent_pnum = 0;

	lpage.internal_page.parent_pnum = root_pnum;
	rpage.internal_page.parent_pnum = root_pnum;
	file_write_page(root_pnum, &root_page);
	file_write_page(l_pagenum, &lpage);
	file_write_page(r_pagenum, &rpage);

	page_t hpage = get_root_page();
	hpage.header_page.root_pnum = root_pnum;
	file_write_page(header_page_num, &hpage);
}



/* First insertion:
 * start a new tree.
 */
void start_new_tree(uint64_t key, char* value) {
	
	page_t hpage = get_header_page();

	page_t rpage;
	pagenum_t r_pnum = file_alloc_page();
	hpage = get_header_page();
	hpage.header_page.root_pnum = r_pnum;
	rpage.leaf_page.parent_pnum = 0;
	rpage.leaf_page.is_leaf = 1;
	rpage.leaf_page.num_of_k = 1;
	rpage.leaf_page.r_pnum = 0;
	rpage.leaf_page.records[0].key = key;
	strcpy(rpage.leaf_page.records[0].value, value);
	file_write_page(r_pnum, &rpage);
	file_write_page(header_page_num, &hpage);
	
}

int index_find(uint64_t key, char* ret_val)
{	
	
	int i = 0;
	pagenum_t pnum;
	page_t lpage = find_leaf(key, &pnum);
	
	while( i < lpage.leaf_page.num_of_k)
	{

		
			if (key > lpage.leaf_page.records[i].key) i++;
			else if(key == lpage.leaf_page.records[i].key)
			{
				ret_val = lpage.leaf_page.records[i].value;
				return i;
			}
			else break;
		
	}




	ret_val = NULL;	
	
	
	return -1;
}

int index_find_2(uint64_t key)
{	
	
	int i = 0;
	pagenum_t pnum;
	page_t lpage = find_leaf(key, &pnum);
	
	while( i < lpage.leaf_page.num_of_k)
	{

		if(lpage.leaf_page.is_leaf == 1){
			if (key > lpage.leaf_page.records[i].key) i++;
			else if(key == lpage.leaf_page.records[i].key)
			{
				return i;
			}
			else break;
		}
		else
		{
			if (key > lpage.internal_page.keys[i].key) i++;
			else if(key == lpage.internal_page.keys[i].key)
			{
				return i;
			}
			else break;
		}
	}

	return -1;


}
/* Master insertion function.
 * Inserts a key and an associated value into
 * the B+ tree, causing the tree to be adjusted
 * however necessary to maintain the B+ tree
 * properties.
 */
int insert(uint64_t key, char* value ) {
	
	page_t hpage = get_header_page();
	
    pagenum_t pagenum;
    page_t lpage = make_leaf_page();
	char temp;
	char* ptemp;				//나중에오류생길수도있음 동적할당??
    /* The current implementation ignores
     * duplicates.
     */
	if (hpage.header_page.root_pnum == 0) {
		start_new_tree(key, value);
	
		return 0;
	}
	


    if (index_find(key, ptemp) != -1){
		return -1;
	}  //insertion fail

    /* Create a new record for the
     * value.
     */

    /* Case: the tree does not exist yet.
     * Start a new tree.
     */


	
    /* Case: the tree already exists.
     * (Rest of function body.)
     */
	
    lpage = find_leaf(key, &pagenum);

    /* Case: leaf has room for key and pointer.
     */
	int temp1;
    if (lpage.leaf_page.num_of_k < leaf_order - 1) {
        temp1 = insert_into_leaf(pagenum, lpage, key, value);
		file_read_page(pagenum, &lpage);
		

        return 0;
    }


    /* Case:  leaf must be split.
     */

    insert_into_leaf_after_splitting(pagenum, lpage, key, value);
	return 0;
}





//deletion//

void remove_entry_from_page(int key){
	pagenum_t pagenum;
	page_t lpage = find_leaf(key, &pagenum);
	
	int i, deletion_point;
	deletion_point = 0;
	while(lpage.leaf_page.records[deletion_point].key =! key) deletion_point++;
	for(i = deletion_point+1; i <lpage.leaf_page.num_of_k; i++){
	
		lpage.leaf_page.records[i - 1] = lpage.leaf_page.records[i];
	}	
	lpage.leaf_page.num_of_k--;
}


int adjust_root()
{
	page_t npage = make_internal_page();
	page_t rpage = get_root_page();
	pagenum_t n_pnum;	
	if(rpage.internal_page.num_of_k > 0) return 0;
	if(rpage.internal_page.is_leaf ==0)
	{
		n_pnum = rpage.internal_page.l_pnum;
		file_read_page(n_pnum, &npage);
		npage.internal_page.parent_pnum = 0;
	}
	else{
	page_t hpage = get_header_page();
	pagenum_t temp_pnum = hpage.header_page.root_pnum;	
	hpage.header_page.root_pnum = 0;
	file_free_page(temp_pnum);
	}
	return 0;
}


int redistribute_pages(pagenum_t p_pnum, int index, int neighbor_index)
{
	if(neighbor_index == 1)
	{
	
	}
	return 0;
}
int delete_entry(uint64_t key)
{
	char abc;
	char* pabc = &abc;
	pagenum_t pagenum;	//page의 pagenum
	page_t hpage = get_header_page();
	page_t page = find_leaf(key, &pagenum);
	remove_entry_from_page(key);

	if(hpage.header_page.root_pnum == pagenum)
	{
		adjust_root();	
		return 0;
	}
	int neighbor;
	page_t ppage;
	pagenum_t p_pnum = page.internal_page.parent_pnum;
	file_write_page(p_pnum, &ppage);
	int index = index_find_2(key);
	if(index = 0) neighbor = 1;
	if(page.internal_page.num_of_k == 1) redistribute_pages(p_pnum, index, neighbor);
	return 0;
}





int delete(uint64_t key) {

	char temp;
	char* ptemp = &temp;


	 if (index_find(key, ptemp) == -1){
		return -1;
	}  
	delete_entry(key);
	return 0;

}






