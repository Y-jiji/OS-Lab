#include "syslib.h"

int sys_chrt(
    endpoint_t proc_ep,
    uint32_t ddl
) {
    message m;
    m.m_u32.data[0] = proc_ep;
    m.m_u32.data[1] = ddl;
    return (_kernel_call(SYS_CHRT, &m));
}
