// pointer arithmetics

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
		int locvar;
		locvar = ch + 1;
		*p = ch;
		ch = locvar;
		p = p + 1;
	}
	*p = 0;
	printf("%s\n", arr);
}


int main(int argc, char** argv)
{
	pointerarithmeticstest();
	return 0;
}
