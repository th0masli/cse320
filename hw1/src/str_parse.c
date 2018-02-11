#include <stdio.h>

/*compare 2 strings*/
int str_cmp(char *str0, char *str1) {
	while (*str0 && *str0==*str1) /*compare the first char of the strings*/
	{
		str0++; /*increase address*/
		str1++;
	}

	if (*str0 == *str1)
		return 0;
	else
		return 1;
}

/*return the length of the string*/
int slen(char *str) {
	int len = 0;
	while (*str) {
		len++; /*increase length*/
		str++; /*increase address*/
	}

	return len;
}

/*convert string to hex int*/
int str_hex(char *str) {
	int num, temp;
	num = 0;
	while (*str) {
		if (*str >= 'A' && *str <= 'F')
			temp = *str - 'A' + 10;
		else if (*str >= 'a' && *str <= 'f')
			temp = *str -'a' + 10;
		else
			temp = *str - '0';
		num = num*16;
		num = num + temp;

		str++;
	}

	return num;
}
