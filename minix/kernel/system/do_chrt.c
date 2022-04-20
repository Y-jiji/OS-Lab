#include "kernel/system.h"
#include <minix/endpoint.h>
#include "kernel/clock.h"

/*===========================================================================*
 *				do_chrt				     *
 *===========================================================================*/
int do_chrt(struct proc * caller, message * m_ptr) {
    int proc_nr;
    /* check and get proc_nr */
    if (!isokendpt(m_ptr->m_u32.data[0], &proc_nr))
        return EINVAL;
    /* get proc, change ddl */
    struct proc* p = proc_addr(proc_nr);
    p->ddl = m_ptr->m_u32.data[1];
    /* reschedule this process */
    return 0;
}