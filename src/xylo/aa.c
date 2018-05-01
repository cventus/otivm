#include <glapi/core.h>

#include "types.h"
#include "include/aa.h"

enum xylo_aa xylo_get_aa(struct xylo *xylo)
{
	return xylo->aa;
}

void xylo_set_aa(struct xylo *xylo, enum xylo_aa aa)
{
	xylo->aa = aa;
}
