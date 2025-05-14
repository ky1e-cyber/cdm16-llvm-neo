#include <stdarg.h>

long long sum(int Count, ...) {
  long long Result = 0;

  va_list Args;
  va_start(Args, Count);

  Result += va_arg(Args, long long);
  Result += va_arg(Args, long);
  Result += va_arg(Args, int);

  va_end(Args);
  return Result;
}

int main(void) { return sum(8, 3ll, 4l, 5); }
