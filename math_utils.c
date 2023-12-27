#include <math.h>
#include <stdlib.h>

#include "math_utils.h"

int getNumDigits(const int n) {
    return (int)(log10(abs(n))) + 1;
}

/*  Based on http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html
 *  Octile/Euclydean    Manhattan        Chebyshev
 *      √2 1 √2          2  1  2          1  1  1
 *      1  H  1          1  H  1          1  H  1
 *      √2 1 √2          2  1  2          1  1  1
 */
double getOctileDistance(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return dx + dy + (sqrt(2) - 2) * min(dx, dy);
}

uint16_t getManhattanDistance(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return dx + dy;
}

uint16_t getChebyshevDistance(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return dx + dy - min(dx, dy);
}

bool areAdjacent(const Int16Vector2 a, const Int16Vector2 b) {
    return getChebyshevDistance(a, b) == 1;
}
