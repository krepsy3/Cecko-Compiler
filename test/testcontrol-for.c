// if statement

void f(_Bool a, _Bool b, _Bool c)
{
	_Bool d;
	for (d = 0; a; c = d)
	{
		a = b;
		b = c;
		printf("looping ");
	}
	printf("done\n");
}

int main(int argc, char** argv)
{
	f(0, 0, 0);
	f(0, 0, 1);
	f(0, 1, 0);
	f(0, 1, 1);
	f(1, 0, 0);
	f(1, 0, 1);
	f(1, 1, 0);
	f(1, 1, 1);
	int i;
	for (i = 0; i < 10; i = i + 1)
	{
		printf("%d ", i);
	}
	printf("done\n");
	return 0;
}
