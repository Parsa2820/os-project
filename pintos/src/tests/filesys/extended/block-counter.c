
#include "tests/lib.h"
#include "tests/main.h"
#include "tests/userprog/sample.inc"
#include <random.h>
#include <syscall.h>

int FILE_SIZE = 64000;


void test_main(void)
{
    int handle, byte_cnt;
    char buffer;
    char file;
    CHECK(create("test.txt", FILE_SIZE), "create \"test.txt\"");
    CHECK((handle = open("test.txt")) > 1, "open \"test.txt\"");
    reset_write_read_cnt();
    for (int i = 0; i < FILE_SIZE; i++)
    {
        file = (i % 30) + 97;
        byte_cnt = write(handle, &file, 1);
        if (byte_cnt != 1)
            fail("write() returned %d instead of %zu", byte_cnt, 1);
    }
    close(handle);
    handle = open("test.txt");
    for (int i = 0; i < FILE_SIZE; i++)
    {
        byte_cnt = read(handle, &buffer, 1);
        if (byte_cnt != 1)
            fail("read() returned %d instead of %zu", byte_cnt, 1);
    }
    close(handle);
    int read_cnt = get_read_cnt();
    int write_cnt = get_write_cnt();
    if (write_cnt < 128 + 10 && write_cnt > 128 - 10){
        msg("write_cnt is reasonable!");
    }
}
