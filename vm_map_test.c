#include "test.h"
#include "stdio.h"

#include <assert.h>

#include "vm_map_map.h"

// setting/getting page

void push_vm_new_page() {    
    printf("page init...\n");
    vm_new_page* p = vm_new_page_init((void*)1);
    p->phys_addr = 123;

    printf("adding page...\n");
    set_new_page(p);
    
    printf("getting page...\n");
    vm_new_page p2 = get_new_page((void*)1);

    assert(p->phys_addr == p2.phys_addr);
    assert((void*)&p != (void*)&p2);
    printf("page init test is completed\n\n");
}


// getting page that does not exist

void test2() {
    printf("checking empty page...\n");
    vm_new_page p3 = get_new_page((void*)123);
    assert(p3.exists == 0);
    assert(p3.virt_addr == 0);
    assert(p3.phys_addr == 0);
    printf("empty page test is completed\n\n");
    
}


// check hit/miss rate

void test3(int total) {
    printf("counting hits/total rate...\n");
    int hits = 0;
    u_int64_t v_addr;
    for (v_addr = 0; v_addr < total; v_addr++) {
        vm_new_page* p = vm_new_page_init((void*)v_addr);
        p->phys_addr = v_addr ^ 123;
        set_new_page(p);
    }
    for (v_addr = 0; v_addr < total; v_addr++) {
        vm_new_page p = get_new_page((void*)v_addr);
        if (p.exists == 1 && (u_int64_t)p.virt_addr == v_addr) {
            assert(p.phys_addr == (v_addr ^ 123));
            hits++;
        }
    }
    printf("hits/total %d/%d\n\n", hits, total);
}

void phys_addr_set(vm_new_page *p) {
    p->phys_addr = 100;
}

static int count = 0;

void phys_addr_count(vm_new_page *p) {
    if (p->phys_addr == 100)
        count++;
}

// change every phys addr to a constant

void update_all_test() {
    printf("update all test...\n");
    vm_map_map_do_for_all(phys_addr_set, 0);
    vm_map_map_do_for_all(phys_addr_count, 1);
    assert(count == LEN);
    printf("update all test is complited.\n\n");
}


int do_test_vm_map(const char *test_parm)
{
    printf("\n\nvm page testing started...\n\n");

    printf("vm_map init...\n");
    vm_map_map_init();

    push_vm_new_page();
    test2();
    test3(2000);

    update_all_test();

    printf("Tests are complited.\n");
    // printf("Dummy vm map test\n")
    return 0;
}
