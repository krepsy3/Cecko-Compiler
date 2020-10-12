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

void stringtest(void)
{
	char arr[100];
	int i;
	char t[40];
	printf("... stringtest ...\n");
	sprintf( arr, "%d %s", 1, "text");
	printf("sprintf: %s\n", arr);
	i = -1;
	t[0] = 0;
	sscanf( arr, "%d%s", &i, t);
	printf("sscanf: %d %s\n", i, t);
	/*
	printf("Enter a number\n");
	scanf("%d", &i);
	printf("The number was %d\n", i);
	*/
	memset(t, 'X', 39);
	t[39] = 0;
	printf("%s\n", t);
}

void pointerarithmeticstest(void)
{
	char arr[27];
	char* p;
	char* e;
	char ch;
	printf("... pointerarithmeticstest ...\n");
	ch = 'A';
	p = arr;
	e = arr + 26;
	while (p != e)
	{
		*p = ch;
		ch = ch + 1;
		p = p + 1;
	}
	*p = 0;
	printf("%s\n", arr);
}

void argreverttest(int argc, char ** argv)
{
	const char* z;
	int i;
	printf("... argreverttest ...\n");
	printf("sizeof(struct Str) = %d\n", sizeof(struct Str));
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
}

void fibtest(void)
{
	int n;
	int s;
	printf("... fibtest ...\n");
	n = 20;
	s = fib(n);
	printf("fib(%d) returned %d\n", n, s);
}

_Bool status;

_Bool test(void)
{
	return status;
}

int main(int argc, char** argv)
{
	status = 0;
	printf("This is test1.c main()\n");
	argreverttest(argc, argv);
	fibtest();
	stringtest();
	pointerarithmeticstest();
	if (test())
	{
		printf("Going to die\n");
		printf("DEATH=%s\n", 999999999);
	}
	return 0;
}