#include <sys/cdefs.h>
#include "namespace.h"
#include <lib.h>

#include <string.h>
#include <unistd.h>
#include <time.h>

int chrt(unsigned long ddl){
    alarm(ddl);
    message m;
    m.m_u32.data[0] = (ddl) ? 
        ddl + (unsigned)(time(NULL))
        : ~0u;
    return _syscall(PM_PROC_NR, PM_CHRT, &m);
}
