#include <math.h>
#include <stdlib.h>

#include "math_utils.h"

int getNumDigits(const int n) {
    return (int)(log10(abs(n))) + 1;
}

// Based on http://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html
double getOctileDistance(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return dx + dy + (sqrt(2) - 2) * min(dx, dy);
}

double getManhattanDistance(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return dx + dy;
}

bool areAdjacent(const Int16Vector2 a, const Int16Vector2 b) {
    const int16_t dx = (int16_t)abs(a.x - b.x);
    const int16_t dy = (int16_t)abs(a.y - b.y);
    return (dx + dy - min(dx, dy)) == 1;
}
