#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int16_t x;
    int16_t y;
} Int16Vector2;

/**
 * Returns the number of digits in an integer.
 * @param n The input integer.
 * @return The number of digits in the input integer.
 */
int getNumDigits(int n);

/**
 * Calculates the octile distance between two points in a grid.
 * @param a The first point (Int16Vector2).
 * @param b The second point (Int16Vector2).
 * @return The octile distance between points a and b.
 */
double getOctileDistance(Int16Vector2 a, Int16Vector2 b);

/**
 * Calculates the Manhattan distance between two points in a grid.
 * @param a The first point (Int16Vector2).
 * @param b The second point (Int16Vector2).
 * @return The Manhattan distance between points a and b.
 */
uint16_t getManhattanDistance(Int16Vector2 a, Int16Vector2 b);

/**
 * Calculates the Chebyshev distance between two points in a grid.
 * @param a The first point (Int16Vector2).
 * @param b The second point (Int16Vector2).
 * @return The Chebyshev distance between points a and b.
 */
uint16_t getChebyshevDistance(Int16Vector2 a, Int16Vector2 b);

/**
 * Checks if two points are adjacent in a grid.
 * @param a The first point (Int16Vector2).
 * @param b The second point (Int16Vector2).
 * @return True if points a and b are adjacent, false otherwise.
 */
bool areAdjacent(Int16Vector2 a, Int16Vector2 b);
