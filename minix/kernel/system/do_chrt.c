#include "kernel/system.h"
#include <minix/endpoint.h>
#include "kernel/clock.h"

/*===========================================================================*
 *				do_chrt				     *
 *===========================================================================*/
int do_chrt(struct proc * caller, message * m_ptr) {
    struct proc* p;
    int proc_nr;
    /* check and get proc_nr */
    if (!isokendpt(m_ptr->m_u32.data[0], &proc_nr))
        return EINVAL;
    p = proc_addr(proc_nr);
    /* change ddl */
    p->ddl = m_ptr->m_u32.data[1];
    /* finally reschedule this process */
    return sched_proc(p, 5, -1, -1);
}