#ifndef ASSEMBLER_MACROS_H
#define ASSEMBLER_MACROS_H

#include "types.h"

// trying to expend macros, if succeeds puts the expended source in 'new_source'
// returns true in expended else false
bool try_expend_macros(String src, String* new_source);

#endif //ASSEMBLER_MACROS_H
