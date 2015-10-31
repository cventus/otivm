
/* Temporary stack. Use it to store temporary results that need to be cleaned
   up in case of an error. The stack is temporary also in the sense that it
   should only be used temporarily, since it can only grow. */
struct tstack
{
	struct wbuf buf;
	jmp_buf *failjmp;
};

/* Initialize a new resource stack */
void tstack_init(struct tstack *ts, jmp_buf *failjmp);

/* Initialize a new failure stack an initialize it to hold a minimum of `n`
   entries. */
void tstack_initn(struct tstack *ts, jmp_buf *failjmp, size_t n);

/* Add a new item to the failure stack `ts`. When a failure occurs, or the
   stack resources are freed, call `dtor` with `p` and `context` as arguments
   (in that order). If `p` is NULL, then fail automatically. */
void tstack_push(
	struct tstack *ts,
	void (*dtor)(void const *, void const *),
	void const *p,
	void const *context);

/* Retain the resource `p` points, i.e. don't call its destuctor when freed
   the stack. This is typically called just before returning from a function to
   mark the resources that should be returned so that temporary resources can
   be destroyed. Return zero on success or non-zero on failure, e.g. resource
   not found in the stack, or it has already been retained, etc. */
int tstack_retain(
	struct tstack *ts,
	void (*dtor)(void const *, void const *),
	void const *p);

/* Retain all resources, i.e. don't call any destuctor registered so far when
   freeing the stack. This is typically called just before returning from a
   function to signify that everything went well. */
void tstack_retain_all(struct tstack *ts);

/* Push a pointer returned by `malloc(3)` onto the stack. When the stack is
   freed, the pointer is passed to `free(3)`. */
void tstack_push_mem(struct tstack *ts, void const *p);
int tstack_retain_mem(struct tstack *ts, void const *p);

/* Push a file stream onto the stack. When the stack is freed, the stream is
   closed with `fclose(3)`. */
void tstack_push_file(struct tstack *ts, FILE *fp);
int tstack_retain_file(struct tstack *ts, FILE *fp);

/* Push a write buffer onto the stack. When the stack is freed, the pointer
   is passed to `wbuf_free()`. */
void tstack_push_wbuf(struct tstack *ts, struct wbuf *buf);
int tstack_retain_wbuf(struct tstack *ts, struct wbuf *buf);

/* Free all resources on the stack, calling their destructors, and free up all
   memory referenced by the stack. */
void tstack_free(struct tstack *ts);

/* Free all resources on the stack, calling their destructors, and free up all
   memory referenced by the stack (like tstack_free()). Then `longjmp(3)` to
   the error location. */
void tstack_fail(struct tstack *ts);

