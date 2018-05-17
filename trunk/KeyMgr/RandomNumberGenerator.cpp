#include "RandomNumberGenerator.h"
#include <unistd.h>

RandomNumberGenerator::RandomNumberGenerator(unsigned int w, unsigned int z)
{
	seed(w, z);
}

RandomNumberGenerator::~RandomNumberGenerator(void)
{
}

void RandomNumberGenerator::seed(unsigned int w, unsigned int z)
{
	m_w = w;    /* must not be zero */
	m_z = z;    /* must not be zero */
}
unsigned int RandomNumberGenerator::get_random_number()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}
