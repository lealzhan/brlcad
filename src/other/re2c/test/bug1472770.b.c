/* Generated by re2c */
#line 1 "bug1472770.b.re"
#define NULL ((char*) 0)
#define YYCTYPE char
#define YYCURSOR p
#define YYLIMIT p
#define YYMARKER q
#define YYFILL(n)

#include <stdio.h>

int scan(char *p)
{
	int n = 0;
	char *q;
	
	printf("[--------------\n");
	printf("%s\n", p);
	printf("]--------------\n");
start:

#line 23 "<stdout>"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0, 128, 128, 128, 128, 128, 128, 128, 
		128, 128,   0, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
		128, 128, 128, 128, 128, 128, 128, 128, 
	};
	goto yy0;
yy1:
	++YYCURSOR;
yy0:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy1;
	}
	if (yych <= 0x00) goto yy5;
	++YYCURSOR;
#line 21 "bug1472770.b.re"
	{
		++n;
		goto start;
	}
#line 76 "<stdout>"
yy5:
	++YYCURSOR;
#line 26 "bug1472770.b.re"
	{
		return n;
	}
#line 83 "<stdout>"
}
#line 29 "bug1472770.b.re"

}

int main(int argc, char **argv)
{
	int n = 0;
	char *largv[4];

	if (argc < 2)
	{
		argc = 4;
		argv = largv;
		argv[1] = "";
		argv[2] = "1\n\n";
		argv[3] = "1\n2\n";
	}
	while(++n < argc)
	{
		printf("%d\n", scan(argv[n]));
	}
	return 0;
}
