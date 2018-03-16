#pragma once

class RandomNumberGenerator
{
public:
	RandomNumberGenerator(unsigned int w, unsigned int z);
	virtual ~RandomNumberGenerator(void);
	void seed(unsigned int w, unsigned int z);
	unsigned int get_random_number();

private:
	unsigned int m_w, m_z;
};
