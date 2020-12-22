// implicit conversion to _Bool

int btoi(_Bool p)
{
	return p;
}

int* pi;

int main(int argc, char** argv)
{
	{
		int i;
		const char * px, * py, * pz;
		_Bool a, b, c, d, e, f;
		int g;

		px = "ABCDEFGHIJK";
		py = px + 3;
		pz = px + 7;

		a = py < pz;
		b = pz <= py;
		c = px >= px;
		d = py > pz;
		e = pi != &i;
		pi = &i;
		f = pi == &i;
		g = py - pz;

		printf("a=%d b=%d c=%d d=%d e=%d f=%d g=%d\n", btoi(a), btoi(b), btoi(c), btoi(d), btoi(e), btoi(f), g);
	}
	return 0;
}
