#include <stdlib.h>
#include <string.h>


#include <vm_map_map.h>

static vm_page*            vm_map_begin;        // array of pages
static vm_page*            vm_map_end;          // a byte after map

// static hal_mutex_t         vm_map_mutex;
static pthread_mutex_t     vm_map_mutex;

// to allocate 2^20 * 4kb = 4gb
#define LEN 16 //1024 * 1024

static void vm_map_map_init() 
{
    int mapsize = LEN * sizeof(vm_page);
    vm_map_begin = (vm_page*) malloc(mapsize);
    memset(vm_map_begin, 0, mapsize);
    vm_map_end = vm_map_begin + LEN;


    pthread_mutex_init(&vm_map_mutex, NULL);
    // hal_mutex_init(&vm_map_mutex, "VM Map");
}

vm_page* vm_page_init(void *v_addr)
{
    vm_page* p = (vm_page*) malloc(sizeof(vm_page));
    memset(p, 0, sizeof(vm_page));
    p->virt_addr = v_addr;
    pthread_mutex_init(&p->lock, NULL);
    return p;
}


static inline u_int64_t hash(u_int64_t val) 
{
    return val % LEN;
}


// updates vm_page value in vm_map_map structure by val->virt_addr

static void set_page(vm_page* val) 
{
    int page_idx = hash((u_int64_t)val->virt_addr);
    vm_page* p = vm_map_begin + page_idx;

    // hal_mutex_lock(&vm_map_mutex);
    pthread_mutex_lock(&vm_map_mutex);
    pthread_mutex_lock(&p->lock);
    *p = *val;
    p->phys_addr = page_idx;
    pthread_mutex_unlock(&p->lock);
    pthread_mutex_unlock(&vm_map_mutex);
}


// returns vm_page value from vm_map_map structure by its v_addr

static vm_page get_page(void* v_addr) 
{
    int page_idx = hash((u_int64_t)(v_addr));   
    if (vm_map_begin[page_idx].virt_addr != v_addr)
        return (const struct vm_page){0};

    vm_page* p = vm_map_begin + page_idx;
    pthread_mutex_lock(&p->lock);
    vm_page res = *(p); 
    pthread_mutex_unlock(&p->lock);
    return res;
}





// Used to show progress
static int vm_map_do_for_percentage = 0;

static void vm_map_do_for_all(vmem_page_func_t func, int lock)
{
    u_int64_t total = vm_map_end - vm_map_begin;
    vm_page *i;

    pthread_mutex_lock(&vm_map_mutex);
    for (i = vm_map_begin; i < vm_map_end; i++)
    {
        if (!i->exists)
            continue;

        if (lock)
            pthread_mutex_lock(&i->lock);
        // else
            // assert(!hal_mutex_is_locked(&i->lock));
        
        func( i );
        
        if (lock)
            pthread_mutex_unlock(&i->lock);
        vm_map_do_for_percentage = (100L*(i-vm_map_begin))/total;
    }
    vm_map_do_for_percentage = 100;
    pthread_mutex_unlock(&vm_map_mutex);
}

#include <stdio.h>
#include <assert.h>

int main(){
    printf("vm_map init...\n");
    vm_map_map_init();


    // setting/getting page
    printf("page init...\n");
    vm_page* p = vm_page_init((void*)1);
    printf("page address %p\n", p);
    p->some_value = 123;
    printf("page initialized.\n");


    printf("adding page...\n");
    set_page(p);
    
    printf("getting page...\n");
    vm_page p2 = get_page((void*)1);

    assert(p->some_value == p2.some_value);
    assert((void*)&p != (void*)&p2);


    // getting page that do not exist

    vm_page p3 = get_page((void*)123);
    assert(p3.exists == 0);
    assert(p3.virt_addr == 0);
    assert(p3.phys_addr == 0);
    // assert(p3.lock == 0);


    printf("test complited.\n");
    
}