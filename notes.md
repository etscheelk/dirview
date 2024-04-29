```c
// Never seen struct created like this before
struct
{
	char *name;
	int val;

	TickitPen *pen_fg, *pen_fg_hi, *pen_bg, *pen_bg_hi;
} penColors[] = 
{
	{"blue", 1},
	{"red", 1}
};
```