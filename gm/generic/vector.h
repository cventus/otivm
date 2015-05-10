#ifndef L
#error "Vector length L not defined!"
#endif
#ifndef VECTOR_TAG
#define VECTOR_TAG PASTE(v,L)
#endif
#undef VECTOR_
#define VECTOR_(tag,name) PASTE(tag,MANGLE(name))
#undef V
#define V(name) VECTOR_(VECTOR_TAG,name)
