// if statement

void f(_Bool a, _Bool b, _Bool c)
{
	_Bool p, q, r;
	p = a;
	q = b;
	r = c;
	while (a)
	{
		a = b;
		b = c;
		c = 0;
		printf("running ");
	}
	do {
		p = q;
		q = r;
		r = 0;
		printf("doing ");
	} while (p);
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
	return 0;
}
