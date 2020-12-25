// fibbonacci

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

void fibtest(void)
{
	int n;
	int s;
	printf("... fibtest ...\n");
	n = 20;
	s = fib(n);
	printf("fib(%d) returned %d\n", n, s);
}

int main(int argc, char** argv)
{
	fibtest();
	return 0;
}
