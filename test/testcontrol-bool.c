// implicit conversion to _Bool

int btoi(_Bool p)
{
	return p;
}

_Bool itob(int p)
{
	return p;
}

char* px;

int main(int argc, char** argv)
{
	{
		int i;
		char x;
		_Bool a, b, c, d, e, f, g, h;

		i = 729;
		a = i;
		i = 0;
		b = i;
		x = 'X';
		c = x;
		x = 0;
		d = x;
		e = px;
		px = &x;
		f = px;
		g = itob(-1);
		h = !g;

		printf("a=%d b=%d c=%d d=%d e=%d f=%d g=%d h=%d\n", btoi(a), btoi(b), btoi(c), btoi(d), btoi(e), btoi(f), btoi(g), btoi(h));
	}
	return 0;
}
