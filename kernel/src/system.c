
#include <types.h>
int isdigit(int c) {
	if (c >= '0' && c <= '9')
		return 1;
	else
		return 0;
}

/* Calculates base^exp - integers only. No negative exponents. */
int ipow(int base, int exp) {
	if (base == 0)
		return 0; // 0^anything is 0
	if (exp < 0)
		return 0; // we don't support negative exponents
	if (exp == 0)
		return 1; // anything^0 is 1 (except 0^0, handled above)

	int result = 1;
	while (exp--)
		result *= base;

	return result;
}

/* Converts a char array to an integer. */
int atoi(const char *str) {
	if (*str == 0)
		return 0;

	int num = 0;

	/* skip non-number data */
	while (*str && !isdigit(*str) && *str != '-') str++;

	/* support negative numbers */
	int sign = 1;
	if (*str == '-') {
		sign = -1;
		str++;
	}

	/* calculate the end of the actual number data (other data is allowed and ignored) */
	const char *end = str;
	while (isdigit(*end)) end++;

	/* calculate the length */
	size_t len = (end - str);

	/* Calculate the actual number */
	const char *p = str;
	for (size_t i = 0; i < len; i++) {
		num += (*p++ - 0x30) * ipow(10, (len - i - 1));
	}

	/* Flip the sign (multiply by -1) if needed */
	num *= sign;

	return num;
}
