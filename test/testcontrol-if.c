// if statement

void f(_Bool a, _Bool b, _Bool c)
{
	if (a)
	{
		printf("a is true ");
	}
	if (b)
	{
		printf("b is true ");
		if (c)
		{
			printf("c is also true ");
		}
	}
	else
	{
		printf("b is false ");
		if (c)
		{
			printf("c is true ");
		}
		else
		{
			printf("c is also false ");
		}
	}
	printf("\n");
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
	return 0;
}
