INCLUDE_DIRECTORIES(. ../../)
ADD_DEFINITIONS(-DNO_SYS_HEADERS -DDEBUG)
ADD_LIBRARY(x86emucore STATIC debug.c decode.c fpu.c ops2.c ops.c prim_ops.c printk.c sys.c)
