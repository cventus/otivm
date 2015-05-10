#define GENERIC_H
#ifndef T
#error "Type T is not defined!"
#endif
#ifndef S
#error "Function suffix letter S is not defined!"
#endif
#ifndef F
#error "Floating point literal suffix F is not defined!"
#endif
#ifndef FPFX
#error "Macro prefix FPFX is not defined!"
#endif
#ifndef PRINTF_FORMAT
#error "printf() format PRINTF_FORMAT is not defined!"
#endif
#ifndef SCANF_FORMAT
#error "scanf() format SCANF_FORMAT is not defined!"
#endif
#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define PASTE_(a,b) a ## b
#define PASTE(a,b) PASTE_(a,b)
#define MANGLE(f) PASTE(f,S)
#define LIT(x) PASTE(x,F)
#define PFMTW(width,conv) "%" width PRINTF_FORMAT conv
#define PFMTEW(width) PFMTW(width,"e")
#define PFMTFW(width) PFMTW(width,"f")
#define PFMTGW(width) PFMTW(width,"g")
#define PFMTE PFMTEW("")
#define PFMTF PFMTFW("")
#define PFMTG PFMTGW("")
#define SFMTW(width,conv) "%" width SCANF_FORMAT conv
#define SFMTFW(width) SFMTW(width,"f")
#define SFMT SFMTFW("")
#define MIN PASTE(FPFX,_MIN)
#define MAX PASTE(FPFX,_MAX)
#ifndef EPSILON
#define EPSILON LIT(1e-5)
#endif
#ifndef EQ_REL_EPSILON
#define EQ_REL_EPSILON LIT(1e-6)
#endif
#ifndef EQ_ABS_EPSILON
#define EQ_ABS_EPSILON LIT(1e-8)
#endif
#undef fabsM
#define fabsM MANGLE(fabs)
#undef sinM
#define sinM MANGLE(sin)
#undef asinM
#define asinM MANGLE(asin)
#undef cosM
#define cosM MANGLE(cos)
#undef acosM
#define acosM MANGLE(acos)
#undef tanM
#define tanM MANGLE(tan)
#undef atanM
#define atanM MANGLE(atan)
#undef atan2M
#define atan2M MANGLE(atan2)
#undef sqrtM
#define sqrtM MANGLE(sqrt)
#undef fmaxM
#define fmaxM MANGLE(fmax)
#undef fminM
#define fminM MANGLE(fmin)
