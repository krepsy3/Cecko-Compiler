// shortcut evaluation of &&, ||

void f(_Bool a, _Bool b, _Bool c, _Bool d)
{
	printf("((%d&&%d)||(%d&&%d)) = ", a, b, c, d);
	if ((a && b) || (c && d))
	{
		printf("1 ; ");
	}
	else
	{
		printf("0 ; ");
	}
	printf("((%d||%d)&&(%d||%d)) = ", a, b, c, d);
	if ((a || b) && (c || d))
	{
		printf("1\n");
	}
	else
	{
		printf("0\n");
	}
}

int main(int argc, char** argv)
{
	do {
		f(0, 0, 0, 0);
		f(0, 0, 0, 1);
		f(0, 0, 1, 0);
		f(0, 0, 1, 1);
		f(0, 1, 0, 0);
		f(0, 1, 0, 1);
		f(0, 1, 1, 0);
		f(0, 1, 1, 1);
		f(1, 0, 0, 0);
		f(1, 0, 0, 1);
		f(1, 0, 1, 0);
		f(1, 0, 1, 1);
		f(1, 1, 0, 0);
		f(1, 1, 0, 1);
		f(1, 1, 1, 0);
		f(1, 1, 1, 1);
	} while (--argc);

	return 0;
}
