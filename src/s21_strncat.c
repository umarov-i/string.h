#include "s21_string.h"

char *s21_strncat(char *result, const char *str, s21_size_t size) {
  s21_size_t dest_len = s21_strlen(result);
  s21_size_t i;

  for (i = 0; i < size && str[i] != '\0'; i++) {
    result[dest_len + i] = str[i];
  }

  result[dest_len + i] = '\0';

  return result;
}
