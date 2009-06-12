
#ifndef KICKSTART_LIB_H
#define KICKSTART_LIB_H

#include <l4/types.h>
#include "stdbool.h"

static inline bool
is_intersection(L4_Word_t start1, L4_Word_t end1, 
		     L4_Word_t start2, L4_Word_t end2)
{
	return ((start1 >= start2) && (start1 <= end2))
		|| ((end1 >= start2) && (end1 <= end2))
		|| ((start1 <= start2) && (end1 >= end2));
}

#endif // KICKSTART_LIB_H
