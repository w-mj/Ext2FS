#include "mock_disk.h"
#include "mm/buf.h"
#include <stdio.h>

int main(void) {
    Dev::MockDisk disk;
    disk.open(__FILE__);
    MM::Buf buf(128);
    disk.read(buf, 128);
    fwrite(buf.raw(), 1, 128, stdout);
    fflush(stdout);

    disk.seek(0);
    disk.read(buf, 128);
    fwrite(buf.raw(), 1, 128, stdout);
    printf("%d\n", disk.tell());
    fflush(stdout);
    disk.close();
}