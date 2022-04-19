#include "pm.h"

/*===================================*
 *				do_chrt			     *
 *===================================*/

int do_chrt() {
    return sys_chrt(who_e, m_in.m_u32.data[0]);
}