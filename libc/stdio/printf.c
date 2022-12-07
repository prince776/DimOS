#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint-gcc.h>

static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*)data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}

static bool print_decimal(int val)
{
	char str[13];
	str[12] = 0;
	int pos = 11;
	bool negative = val < 0;
	if (val < 0) val *= -1;
	while (pos >= 0)
	{
		str[pos] = '0' + (val % 10);
		val /= 10;
		if (val == 0) break;
		pos--;
	}
	if (negative) {
		str[--pos] = '-';
	}

	int res = printf(str + pos);
	return res != -1;
}

static bool print_unsigned_decimal(unsigned int val)
{
	char str[13];
	str[12] = 0;
	int pos = 11;
	while (pos >= 0)
	{
		str[pos] = '0' + (val % 10);
		val /= 10;
		if (val == 0) break;
		pos--;
	}
	int res = printf(str + pos);
	return res != -1;
}


static bool print_decimal_64(int64_t val)
{
	char str[20];
	str[19] = 0;
	int pos = 18;
	bool negative = val < 0;
	if (val < 0) val *= -1;
	while (pos >= 0)
	{
		str[pos] = '0' + (val % 10);
		val /= 10;
		if (val == 0) break;
		pos--;
	}
	if (negative) {
		str[--pos] = '-';
	}

	int res = printf(str + pos);
	return res != -1;
}

static bool print_unsigned_decimal_64(uint64_t val)
{
	char str[22];
	str[21] = 0;
	int pos = 20;
	while (pos >= 0)
	{
		str[pos] = '0' + (val % 10);
		val /= 10;
		if (val == 0) break;
		pos--;
	}
	int res = printf(str + pos);
	return res != -1;
}

static bool print_hex_64(uint64_t val)
{
	char str[22];
	str[21] = 0;
	int pos = 20;
	while (pos >= 0)
	{
		int x = (val % 16);
		if (x <= 9) {
			str[pos] = '0' + x;
		}
		else {
			x -= 10;
			str[pos] = 'a' + x;
		}
		val /= 16;
		if (val == 0) break;
		pos--;
	}
	pos--;
	str[pos] = 'x';
	pos--;
	str[pos] = '0';
	int res = printf(str + pos);
	return res != -1;
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char)va_arg(parameters, int /* char promotes to int */);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		}
		else if (*format == 's') {
			format++;
			const char* str = va_arg(parameters, const char*);
			size_t len = strlen(str);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'd') {
			format++;
			int d = va_arg(parameters, int);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print_decimal(d))
				return -1;
			written++;
		}
		else if (*format == 'l') {
			format++;
			int64_t d = va_arg(parameters, long long);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print_decimal_64(d))
				return -1;
			written++;
		}
		else if (*format == 'u') {
			format++;
			uint64_t d = (uint64_t)va_arg(parameters, long long);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print_unsigned_decimal_64(d))
				return -1;
			written++;
		}
		else if (*format == 'x') {
			format++;
			uint64_t d = (uint64_t)va_arg(parameters, long long);
			if (!maxrem) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print_hex_64(d))
				return -1;
			written++;
		}
		else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
