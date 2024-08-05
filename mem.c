#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char ALIGN[16];

union header{
    struct{
        size_t size;
        unsigned is_free;
        union header_t *next;
    }s;
    ALIGN stub;
};
typedef union header header_t;

header_t *head=NULL,*tail=NULL;
pthread_mutex_t global_malloc_lock; // global lock to avoid multiple threads accessing memory at the same time

header_t *get_free_block(size_t size)
{
	header_t *curr = head;
	while(curr) {
		if (curr->s.is_free && curr->s.size >= size)
			return curr;
		curr = curr->s.next;
	}
	return NULL;
}

// free function implementation in C
void free(void *block)
{
	header_t *header, *tmp;
	void *programbreak;

	if (!block)
		return;
	pthread_mutex_lock(&global_malloc_lock);
	header = (header_t*)block - 1;

	programbreak = sbrk(0); // current value of the break pointer
	if ((char*)block + header->s.size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->s.next == tail) {
					tmp->s.next = NULL;
					tail = tmp;
				}
				tmp = tmp->s.next;
			}
		}
		sbrk(0 - sizeof(header_t) - header->s.size);
		pthread_mutex_unlock(&global_malloc_lock);
		return;
	}
	header->s.is_free = 1;
	pthread_mutex_unlock(&global_malloc_lock);
}

// implementing malloc(size) function in C
void *malloc(size_t size)
{
    size_t total_size;
    void *mem_block;
    header_t *header;
    if(!size)return NULL;
    pthread_mutex_lock(&global_malloc_lock); // assign the global lock
    header=get_free_block(size); // get the free block
    if(header){
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock); // unlock if exists
        return (void*)(header+1);
    }
    total_size = sizeof(header_t)+size;
    mem_block = sbrk(total_size);
    if(mem_block == (void*)-1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL;
    }
    header=mem_block;
    header->s.size = size;
    header->s.is_free = 0;
    header->s.next = NULL;
    if(!head) head=header;
    if(tail) tail->s.next= header;
    tail=header;
    pthread_mutex_unlock(&global_malloc_lock);
    return (void*)(header+1);
}

// implementing calloc() in C
void *calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	// check mul overflow   
	if (nsize != size / num)
		return NULL;
	block = malloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}

// implementing realloc() in C
void *realloc(void *block, size_t size)
{
	header_t *header;
	void *ret;
	if (!block || !size)
		return malloc(size);
	header = (header_t*)block - 1;
	if (header->s.size >= size)
		return block;
	ret = malloc(size);
	if (ret) {
		
		memcpy(ret, block, header->s.size);
		free(block);
	}
	return ret;
}

//printing the linked list
void printMemory(){
    header_t *now = head;
    printf("HEAD = %p, Tail = %p \n",(void*)head,(void*)tail);
    while(now) {
		printf("addr = %p, size = %zu, is_free=%u, next=%p\n",(void*)now, now->s.size, now->s.is_free, (void*)now->s.next);
		now = now->s.next;
	}
}
