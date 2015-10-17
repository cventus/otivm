
#define CHECK(...) do {\
	if (!(__VA_ARGS__)) { \
		fprintf(stderr, "%s,%d \"%s\": %s\n", __FILE__, __LINE__, \
		        #__VA_ARGS__, strerror(errno)); \
		exit(EXIT_FAILURE); \
	} \
} while (0)

