// linked-list stack

typedef struct Str* str_ptr;

struct Str {
	const char* key;
	str_ptr next;
};

str_ptr root;

struct Str array[1000];
int array_end;

str_ptr get_str(void)
{
	str_ptr p;
	p = &array[array_end];
	array_end = array_end + 1;
	return p;
}

void push_front(str_ptr* rootp, const char* key)
{
	str_ptr p;
	p = get_str();
	(*p).key = key;
	(*p).next = *rootp;
	*rootp = p;
}

_Bool empty(str_ptr* rootp)
{
	return !*rootp;
}

const char* front(str_ptr* rootp)
{
	return (**rootp).key;
}

void pop_front(str_ptr* rootp)
{
	*rootp = (**rootp).next;
}


void argreverttest(int argc, const char* const* argv)
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


int main(int argc, char** argv)
{
	const char* n_argv[5];
	n_argv[0] = "Zero";
	n_argv[1] = "One";
	n_argv[2] = "Two";
	n_argv[3] = "Three";
	n_argv[4] = "Four";
	argreverttest(5, n_argv);
	return 0;
}
