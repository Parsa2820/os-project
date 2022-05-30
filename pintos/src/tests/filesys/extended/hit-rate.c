
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/userprog/sample.inc"
#include <random.h>
#include <syscall.h>

char buffer[sizeof sample - 1];

void test_main(void)
{
    int handle, byte_cnt;

    CHECK(create("test.txt", 6*(sizeof sample - 1)), "create \"test.txt\"");
    CHECK((handle = open("test.txt")) > 1, "open \"test.txt\"");

    for (int i = 0; i < 5; i++)
    {
        byte_cnt = write(handle, sample, sizeof sample - 1);
        if (byte_cnt != sizeof sample - 1)
            fail("write() returned %d instead of %zu", byte_cnt, sizeof sample - 1);
    }
    
    reset_cache();
    close(handle);
    handle = open("test.txt");
    for (int i = 0; i < 5; i++)
    {
        int bytes_read = read(handle, buffer, sizeof sample - 1);
        if (bytes_read != sizeof sample - 1)
            fail("read() returned %d instead of %zu", bytes_read, sizeof sample - 1);
    }

    int cache_miss = get_cache_miss();
    int cache_hit = get_cache_hit();
    int reset_hit_rate = 100 * cache_hit / (cache_hit + cache_miss);
    close(handle);
    handle = open("test.txt");
    for (int i = 0; i < 5; i++)
    {
        byte_cnt = write(handle, sample, sizeof sample - 1);
        if (byte_cnt != sizeof sample - 1)
            fail("write() returned %d instead of %zu", byte_cnt, sizeof sample - 1);
    }

    close(handle);
    handle = open("test.txt");

    for (int i = 0; i < 5; i++)
    {
        int bytes_read = read(handle, buffer, sizeof sample - 1);
        if (bytes_read != sizeof sample - 1)
            fail("read() returned %d instead of %zu", bytes_read, sizeof sample - 1);
    }

    cache_miss = get_cache_miss();
    cache_hit = get_cache_hit();
    int new_hit_rate = 100 * cache_hit / (cache_hit + cache_miss);
    if (new_hit_rate > reset_hit_rate){
        msg("new hit rate is better than old one");
    }else{
        fail("old one is better than new hit rate");
    }
}
