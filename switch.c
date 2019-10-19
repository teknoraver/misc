/*
 
$ gcc -Wswitch -c switch.c -o switch.o
switch.c: In function ‘f1’:
switch.c:15:2: warning: enumeration value ‘QUATTRO’ not handled in switch [-Wswitch]
   15 |  switch (e) {
      |  ^~~~~~
 
*/

enum myenum {
	UNO,
	DUE,
	TRE,
	QUATTRO,
};

#define ONE     1
#define TWO     2
#define THREE   3
#define FOUR    4

int f1(enum myenum e, int x)
{
	switch (e) {
	case UNO:
		return x;
	case DUE:
		return x * 2;
	case TRE:
		return x * 3;
	}

	return 0;
}

int f2(int i, int x)
{
	switch (i) {
	case ONE:
		return x;
	case TWO:
		return x * 2;
	case THREE:
		return x * 3;
	}

	return 0;
}
