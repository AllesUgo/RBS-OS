#include "c_basic.h"

int Strlen(const char *str)
{
	int len = 0;
	for (; str[len]; ++len)
		;
	return len;
}
void Strcpy(char *dst, const char *src)
{
	int i;
	for (i = 0; src[i]; ++i)
	{
		dst[i] = src[i];
	}
	dst[i] = 0;
}
char *Strcat(char *dst, const char *src)
{
	char *p = dst + Strlen(dst);
	Strcpy(p, src);
	return 0;
}

void Memset(void *ptr, int set_num, int buffer_len)
{
	char *end = (char *)ptr + buffer_len;
	char* start = (char*) ptr;
	while (start!=end) *(start++) = set_num;
}
void Memcpy(void *dst, const void *src, int buffer_len)
{
	for (int i = 0; i < buffer_len; ++i)
		((char *)dst)[i] = ((char *)src)[i];
}
char *Itoa(int value, char *str, int base)
{
	char *ptr = str, *start = str;
	int digit;
	char sign = '\0';

	if (value < 0 && base == 10)
	{
		sign = '-';
		value = -value;
	}

	do
	{
		digit = value % base;
		if (digit < 10)
		{
			*ptr++ = digit + '0';
		}
		else
		{
			*ptr++ = digit - 10 + 'a';
		}
		value /= base;
	} while (value > 0);

	if (sign)
	{
		*ptr++ = sign;
	}
	*ptr-- = '\0';

	while (start < ptr)
	{
		char tmp = *start;
		*start++ = *ptr;
		*ptr-- = tmp;
	}

	return str;
}

char* UIntToHex(unsigned int x,char*str)
{
	static const char HEX[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	for (int i=7;i>=0;--i)
	{
		str[i] = HEX[x%16];
		x/=16;
	}
	str[8] = 0;
	return str;
}

void Sprintf(char *buf, const char *fmt, ...)
{
	*buf = 0;
	int ptrsize = sizeof(void *);
	int number_size = sizeof(int);
	int longlongint_size = sizeof(long long int);
	char *base_addr = (char *)&fmt;
	int move_addr = ptrsize;
	char temp[32];
	int dot = 1000000;
	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			switch (*fmt)
			{

			case 'd':
				Itoa(*(int *)(base_addr + move_addr), temp, 10);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += sizeof(int);
				break;
			case 'x':
				Itoa(*(int *)(base_addr + move_addr), temp, 16);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += number_size;
				break;
			case 'b':
				Itoa(*(int *)(base_addr + move_addr), temp, 2);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += number_size;
				break;
			case 's':
				for (int i = 0; *((*(char **)(base_addr + move_addr)) + i); ++i)
					*(buf++) = *((*(char **)(base_addr + move_addr)) + i);
				*buf = 0;
				++fmt;
				move_addr += ptrsize;
				break;
			case '.':
				++fmt;
				if (*fmt > '0' && *fmt <= '9')
				{
					dot = 1;
					for (int i = '0'; i < *fmt; ++i)
						dot *= 10;
					++fmt;
				}
				if (*fmt != 'f')
				{
					dot = 1000000;
					++fmt;
					break;
				}
			case 'f':
			{
				double n = *(double *)(base_addr + move_addr);

				char t[32];
				Itoa(n, t, 10);
				Strcpy(temp, t);
				if (n < 0)
					n = -n;
				n -= (int)n;
				n *= dot;
				int j = 0;
				for (j; dot; dot /= 10)
					++j;
				t[--j] = 0;
				for (int i = j - 1; i >= 0; --i)
				{
					if ((int)n != 0)
					{
						t[i] = (int)n % 10 + '0';
					}
					else
					{
						t[i] = '0';
					}
					if ((int)n != 0)
					{
						n /= 10;
					}
				}
				dot = 1000000;
				Strcat(temp, ".");
				Strcat(temp, t);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += sizeof(double);
				break;
			}
			default:
				++fmt;
				break;
			}
		}
		else
		{
			*buf = *fmt++;
			*(++buf) = 0;
		}
	}
}

/*int Printf(const char*fmt,...)
{
	char *buffer=(char*)Malloc(Strlen(fmt)+1024);
	char *buf=buffer;
	*buf = 0;
	int ptrsize = sizeof(void*);
	int number_size = sizeof(int);
	int longlongint_size = sizeof(long long int);
	char* base_addr = (char*) & fmt;
	int move_addr = ptrsize;
	char temp[32];
	int dot = 1000000;
	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			switch (*fmt)
			{

			case 'd':
				Itoa(*(int*)(base_addr + move_addr), temp, 10);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += sizeof(int);
				break;
			case 'x':
				Itoa(*(int*)(base_addr + move_addr), temp, 16);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += number_size;
				break;
			case 'b':
				Itoa(*(int*)(base_addr + move_addr), temp, 2);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += number_size;
				break;
			case 's':
				for (int i = 0; *((*(char**)(base_addr + move_addr)) + i); ++i)
					*(buf++) = *((*(char**)(base_addr + move_addr)) + i);
				*buf = 0;
				++fmt;
				move_addr += ptrsize;
				break;
			case '.':
				++fmt;
				if (*fmt > '0' && *fmt <= '9')
				{
					dot = 1;
					for (int i = '0'; i < *fmt; ++i)
						dot *= 10;
					++fmt;
				}
				if (*fmt != 'f')
				{
					dot = 1000000;
					++fmt;
					break;
				}
			case 'f':
			{
				double n = *(double*)(base_addr + move_addr);
				char t[32];
				Itoa(n, t, 10);
				Strcpy(temp, t);
				n -= (int)n;
				n *= dot;
				int j=0;
				for (j; dot; dot /= 10) ++j;
				t[--j] = 0;
				for (int i = j - 1; i >= 0; --i)
				{
					if ((int)n != 0)
					{
						t[i] = (int)n % 10 + '0';
					}
					else
					{
						t[i] = '0';
					}
					if ((int)n != 0)
					{
						n /= 10;
					}
				}
				dot = 1000000;
				Strcat(temp, ".");
				Strcat(temp, t);
				for (int i = 0; temp[i]; ++i)
				{
					*buf = temp[i];
					++buf;
				}
				*buf = 0;
				++fmt;
				move_addr += sizeof(double);
				break;
			}
			default:
				++fmt;
				break;
			}
		}
		else
		{
			*buf = *fmt++;
			*(++buf) = 0;
		}
	}
	PrintString(buffer);
	Free(buffer);
}
*/


