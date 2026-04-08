#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <wchar.h>

#include "../s21_string.h"

int opst(va_list *args, ops *op, char **src) {
  int res = 0;
  char *new_str = malloc(sizeof(char));
  if (!new_str) return 0;
  new_str[0] = '\0';
  oksym(src);
  int i = 0;
  int chars_scanned = 0;
  for (; **src && !isspace(**src) && (op->wid == 0 || i < op->wid);
       i++, (*src)++, chars_scanned++) {
    char *tmp = realloc(new_str, (i + 2) * sizeof(char));
    if (!tmp) {
      free(new_str);
      return 0;
    }
    new_str = tmp;
    new_str[i] = **src;
  }
  new_str[i] = '\0';
  if (chars_scanned > 0 && !op->supr) {
    res++;
    if (!op->len) {
      char *dest = va_arg(*args, char *);
      if (dest) s21_strcpy(dest, new_str);
    } else if (op->len == 2) {
      wchar_t *dest = va_arg(*args, wchar_t *);
      if (dest) mbstowcs(dest, new_str, i + 1);
    }
  }
  free(new_str);
  op->count += chars_scanned;
  op->format = 0;
  return res;
}

int s21_sscanf(const char *str, const char *format, ...) {
  int res = 0;
  ops options = {0, 0, 0, 0, 0, 0, 0, '\0'};
  char *tmp_str = malloc((s21_strlen(str) + 1) * sizeof(char));
  if (!tmp_str) {
    return -1;
  }
  s21_strcpy(tmp_str, str);
  char *current_str_ptr = tmp_str;

  va_list args;
  va_start(args, format);

  oksym(&current_str_ptr);

  for (; *format && !options.err; format++) {
    if (*format == '%' && *(format + 1) == '%') {
      format++;
      oksym(&current_str_ptr);
      continue;
    }
    if (optionsin(&current_str_ptr, &options, format)) continue;
    if (options.format) {
      if (isdigit(*format)) {
        options.wid = options.wid * 10 + (*format - '0');
        continue;
      }
      res += processformat(&args, &options, &current_str_ptr, format);
    } else {
      casenon(&current_str_ptr, &options, format);
    }
    if (isbreak(&options, &current_str_ptr)) {
      if (res == 0 && strlen(str) == 0) res = -1;
      break;
    }
  }
  free(tmp_str);
  va_end(args);
  return res;
}

long s21_atoi(char *str, char **strlim, int base) {
  long int res = 0;
  int sign = 1;
  while (isspace(*str)) str++;
  if (*str == '+')
    str++;
  else if (*str == '-') {
    sign = -1;
    str++;
  }
  if (base == 0) {
    if (*str == '0') {
      if (*(str + 1) == 'x' || *(str + 1) == 'X') {
        base = 16;
        str += 2;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  } else if (base == 16 && *str == '0' &&
             (*(str + 1) == 'x' || *(str + 1) == 'X')) {
    str += 2;
  }
  char *digits = "0123456789abcdef";
  for (; *str; str++) {
    char *digit = strchr(digits, tolower(*str));
    int val = digit ? (digit - digits) : -1;
    if (val != -1 && val < base) {
      res = res * base + val;
    } else {
      break;
    }
  }
  if (strlim) *strlim = str;
  return sign * res;
}

unsigned long s21_atoul(char *str, char **strlim, int base) {
  return s21_atoi(str, strlim, base);
}

long double s21_atold(char *str, char **strlim) {
  long double res = 0.0;
  int sign = 1;
  while (isspace(*str)) str++;
  if (*str == '+') {
    str++;
  } else if (*str == '-') {
    sign = -1;
    str++;
  }
  if (strncasecmp(str, "nan", 3) == 0) {
    if (strlim) *strlim = str + 3;
    return NAN * sign;
  }
  if (strncasecmp(str, "inf", 3) == 0) {
    if (strlim) *strlim = str + 3;
    return INFINITY * sign;
  }
  for (; isdigit(*str); str++) {
    res = res * 10.0 + (*str - '0');
  }
  if (*str == '.') {
    str++;
    long double fraction = 0.0;
    long double divisor = 1.0;
    for (; isdigit(*str); str++) {
      fraction = fraction * 10.0 + (*str - '0');
      divisor *= 10.0;
    }
    res += fraction / divisor;
  }
  if (*str == 'e' || *str == 'E') {
    str++;
    int exp_sign = 1;
    if (*str == '+') {
      str++;
    } else if (*str == '-') {
      exp_sign = -1;
      str++;
    }
    int exp = 0;
    for (; isdigit(*str); str++) {
      exp = exp * 10 + (*str - '0');
    }
    res *= powl(10.0, exp * exp_sign);
  }
  if (strlim) *strlim = str;
  return res * sign;
}

double s21_atod(char *str, char **strlim) {
  return (double)s21_atold(str, strlim);
}
float s21_atof(char *str, char **strlim) {
  return (float)s21_atold(str, strlim);
}

int opd(va_list *args, ops *op, char **src, int base) {
  int res = 0;
  char *strlim;
  check_buffer(src, op);
  if (s21_atoi(*src, &strlim, base) != 0 || strlim != *src) {
    if (!op->supr) {
      res++;
      if (op->len == 1)
        *(short int *)va_arg(*args, void *) = s21_atoi(*src, &strlim, base);
      else if (op->len == 2)
        *(long int *)va_arg(*args, void *) = s21_atoi(*src, &strlim, base);
      else
        *(int *)va_arg(*args, void *) = s21_atoi(*src, &strlim, base);
    } else {
      s21_atoi(*src, &strlim, base);
    }
    op->count += strlim - *src;
    *src = strlim;
  } else {
    op->err = 1;
  }
  op->format = 0;
  return res;
}

int opu(va_list *args, ops *op, char **src, int base) {
  int res = 0;
  char *strlim;
  check_buffer(src, op);
  if (s21_atoul(*src, &strlim, base) != 0 || strlim != *src) {
    if (!op->supr) {
      res++;
      if (op->len == 1)
        *(short unsigned *)va_arg(*args, void *) =
            s21_atoul(*src, &strlim, base);
      else if (op->len == 2)
        *(long unsigned *)va_arg(*args, void *) =
            s21_atoul(*src, &strlim, base);
      else
        *(unsigned *)va_arg(*args, void *) = s21_atoul(*src, &strlim, base);
    } else {
      s21_atoul(*src, &strlim, base);
    }
    op->count += strlim - *src;
    *src = strlim;
  } else {
    op->err = 1;
  }
  op->format = 0;
  return res;
}

int opf(va_list *args, ops *op, char **src) {
  int res = 0;
  char *strlim;
  check_buffer(src, op);
  if (s21_atof(*src, &strlim) != 0.0 || strlim != *src) {
    if (!op->supr) {
      res++;
      if (op->len == 3)
        *(long double *)va_arg(*args, void *) = s21_atold(*src, &strlim);
      else if (op->len == 2)
        *(double *)va_arg(*args, void *) = s21_atod(*src, &strlim);
      else
        *(float *)va_arg(*args, void *) = s21_atof(*src, &strlim);
    } else {
      s21_atof(*src, &strlim);
    }
    op->count += strlim - *src;
    *src = strlim;
  } else {
    op->err = 1;
  }
  op->format = 0;
  return res;
}

int opp(va_list *args, ops *op, char **src) {
  int res = 0;
  char *strlim;
  check_buffer(src, op);
  if (s21_atoul(*src, &strlim, 16) != 0 || strlim != *src) {
    if (!op->supr) {
      res++;
      *(void **)va_arg(*args, void **) = (void *)s21_atoul(*src, &strlim, 16);
    } else {
      s21_atoul(*src, &strlim, 16);
    }
    op->count += strlim - *src;
    *src = strlim;
  } else {
    op->err = 1;
  }
  op->format = 0;
  return res;
}

int opc(va_list *args, ops *op, char **src) {
  int res = 0;
  if (!op->supr) {
    res++;
    if (op->len == 2)
      mbtowc(va_arg(*args, wchar_t *), *src, op->wid ? op->wid : 1);
    else
      *(char *)va_arg(*args, char *) = **src;
  }
  (*src)++;
  op->count++;
  op->format = 0;
  return res;
}

void oksym(char **src) {
  while (isspace(**src)) {
    (*src)++;
  }
}

void oppe(char **src, ops *op) {
  if (**src == '%') {
    (*src)++;
  } else {
    op->err = 1;
  }
  op->format = 0;
}

int optionsin(char **src, ops *op, const char *format) {
  int res = 0;
  if (*format == '%' && !op->format) {
    res++;
    op->format = 1;
    op->supr = 0;
    op->len = 0;
    op->wid = 0;
    if (op->buff) {
      *(*src + s21_strlen(*src)) = op->buff;
      op->buff = '\0';
    }
  }
  return res;
}

void opn(va_list *args, ops *op) {
  if (!op->supr) {
    if (op->len == 1)
      *(short int *)va_arg(*args, void *) = op->count;
    else if (op->len == 2)
      *(long int *)va_arg(*args, void *) = op->count;
    else
      *(int *)va_arg(*args, void *) = op->count;
  }
  op->format = 0;
}

void casenon(char **src, ops *op, const char *format) {
  if (isspace(*format)) {
    oksym(src);
  } else if (**src == *format) {
    (*src)++;
    op->count++;
  } else {
    op->err = 1;
  }
}

int isbreak(ops *op, char **src) { return op->err || !**src; }

int processformat(va_list *args, ops *op, char **src, const char *format) {
  int res = 0;
  switch (*format) {
    case 'x':
    case 'X':
      res += opu(args, op, src, 16);
      break;
    case 'p':
      res += opp(args, op, src);
      break;
    case 'e':
    case 'E':
    case 'f':
    case 'g':
    case 'G':
      res += opf(args, op, src);
      break;
    case 'c':
      res += opc(args, op, src);
      break;
    case 'd':
      res += opd(args, op, src, 10);
      break;
    case 'i':
      res += opd(args, op, src, 0);
      break;
    case 'o':
      res += opu(args, op, src, 8);
      break;
    case 's':
      res += opst(args, op, src);
      break;
    case 'u':
      res += opu(args, op, src, 10);
      break;
    case 'n':
      opn(args, op);
      break;
    case '%':
      oppe(src, op);
      break;
    case 'h':
      op->len = 1;
      break;
    case '*':
      op->supr = 1;
      break;
    case 'l':
      op->len = 2;
      break;
    case 'L':
      op->len = 3;
      break;
    default:
      op->err = 1;
  }
  return res;
}

void check_buffer(char **src, ops *op) {
  oksym(src);
  if (op->wid > 0 && op->wid < (int)s21_strlen(*src)) {
    op->buff = *(*src + op->wid);
    *(*src + op->wid) = '\0';
  }
}
