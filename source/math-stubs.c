#include <math.h>

static float fast_cos(float x)
{
	// Adapted from here: https://stackoverflow.com/a/28050328
	x -= 0.25f + floorf(x + 0.25f);
	x *= 16.0f * (fabs(x) - 0.5f);
	x += 0.225f * x * (fabs(x) - 1.0f);
	return x;
}

float sinf(float x)
{
	return fast_cos(x / (2*M_PI) - 0.25f);
}

float cosf(float x)
{
	return fast_cos(x / (2*M_PI));
}
