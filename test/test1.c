int fib(int p);

struct Str {
	const char* key;
	struct Str* next;
};

struct Str* root;

struct Str array[1000];
int array_end;

struct Str* get_str(void)
{
	struct Str* p;
	p = &array[array_end];
	array_end = array_end + 1;
	return p;
}

void push_front(struct Str** rootp, const char* key)
{
	struct Str* p;
	p = get_str();
	(*p).key = key;
	(*p).next = *rootp;
	*rootp = p;
}

_Bool empty(struct Str** rootp)
{
	return !*rootp;
}

const char* front(struct Str** rootp)
{
	return (**rootp).key;
}

void pop_front(struct Str** rootp)
{
	*rootp = (**rootp).next;
}

int fib(int x)
{
	int s;
	if (x <= 2)
		return 1;
	else
	{
		s = fib(x - 1) + fib(x - 2);
		return s;
	}
}

int main(int argc, char** argv)
{
	const char* z;
	int i;
	int n;
	int s;
	i = 0;
	while (i < argc)
	{
		z = argv[i];
		printf("argv[%d] is \"%s\"\n", i, z);
		push_front(&root, z);
		i = i + 1;
	}
	while (!empty(&root))
	{
		z = front(&root);
		pop_front(&root);
		printf("popped \"%s\"\n", z);
	}
	n = 20;
	s = fib(n);
	printf("fib(%d) returned %d\n", n, s);
	return 0;
}