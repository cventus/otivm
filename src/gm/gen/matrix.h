
#define DIAGONAL (M < N ? M : N)

#define MAT_(rows,cols,name) PASTE(m,PASTE(rows,PASTE(cols,MANGLE(name))))
#define MAT(name) MAT_(M,N,name)

