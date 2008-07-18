/* A very fast function to calculate the approximate inverse square root of a
 * floating point value and a helper function that uses it for getting the
 * normal squareroot. For an explanation of the inverse squareroot function
 * read:
 * http://www.math.purdue.edu/~clomont/Math/Papers/2003/InvSqrt.pdf
 *
 * Unfortunately the original creator of this function seems to be unknown.
 *
 * $Id: fastsqrt.h 3508 2007-08-22 16:32:52Z b_lindeijer $
 */

float fastInvSqrt(float x)
{
    union { int i; float x; } tmp;
    float xhalf = 0.5f * x;
    tmp.x = x;
    tmp.i = 0x5f375a86 - (tmp.i >> 1);
    x = tmp.x;
    x = x * (1.5f - xhalf * x * x);
    return x;
}

float fastSqrt(float x)
{
    return 1.0f / fastInvSqrt(x);
}
