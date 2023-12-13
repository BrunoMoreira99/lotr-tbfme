#include <math.h>

#include "math_utils.h"

int getNumDigits(const int n) {
    return (int)(log10(abs(n))) + 1;
}
