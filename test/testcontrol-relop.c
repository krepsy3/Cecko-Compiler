// implicit conversion to _Bool

int btoi(_Bool p)
{
	return p;
}

int main(int argc, char** argv)
{
	{
		int i;
		char x;
		_Bool a, b, c, d, e, f;

		i = 729;
		a = i < 1000;
		i = 0;
		b = 1000 <= i;
		x = 'X';
		c = x >= 'X';
		x = 0;
		d = 'X' > x;
		e = i != 0;
		f = x == x;

		printf("a=%d b=%d c=%d d=%d e=%d f=%d\n", btoi(a), btoi(b), btoi(c), btoi(d), btoi(e), btoi(f));
	}
	return 0;
}
