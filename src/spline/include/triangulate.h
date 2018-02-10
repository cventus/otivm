struct triangle
{
	size_t a, b, c;
};

struct triangulation
{
	size_t n;
	struct triangle triangles[];
};

struct triangulation *triangulate(float const (*vertices)[2], size_t nmemb);
