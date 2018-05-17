#include "PermuteServer.h"
#include "RandomNumberGenerator.h"
#include "stdlib.h"
#include <unistd.h>

#define max(a,b) (((a) > (b))? (a):(b))

static void generate_permutation_table(int w, int z, unsigned short *table, int length)
{
	RandomNumberGenerator rgen(w, z);

	for(unsigned short i=0;i<(unsigned short) length;i++)	
		table[i] = i;

	for(int i=length; i>1; i--)
	{
        unsigned int di = rgen.get_random_number()%i;
		unsigned short tmp = table[di];
		table[di] = table[i-1];
		table[i-1] = tmp;
	}
}

PermuteServer::PermuteServer(int length, unsigned int permutationKey): m_permutationKey(permutationKey), 
m_bitReversePermuteTable(0), m_bitPermuteTable(0), m_bytePermutationTable(0), m_length(length), m_scratch(0)
{
	m_bytePermutationTable = (unsigned short *) malloc(max(256, m_length*2)*sizeof(short));
	m_scratch = (unsigned char *) malloc(m_length*2);

	m_bitPermuteTable = (unsigned char *) malloc(256);
	m_bitReversePermuteTable = (unsigned char *) malloc(256);

	m_permuteRotation = (m_permutationKey & 0x3) + 1;
	m_reversePermuteRotation = 32 - m_permuteRotation;

	generate_permutation_table(m_permutationKey, m_permutationKey+1, m_bytePermutationTable, 256);
	for(int i=0;i<256;i++)
	{
		m_bitPermuteTable[i] = (unsigned char) m_bytePermutationTable[i];
		m_bitReversePermuteTable[m_bitPermuteTable[i]] = i;
	}

	generate_permutation_table(m_permutationKey+1, m_permutationKey, m_bytePermutationTable, m_length*2);
}

PermuteServer::~PermuteServer(void)
{
	if(m_bytePermutationTable)	free(m_bytePermutationTable);	m_bytePermutationTable = 0;
	if(m_bitPermuteTable)	free(m_bitPermuteTable);	m_bitPermuteTable = 0;
	if(m_bitReversePermuteTable)	free(m_bitReversePermuteTable);	m_bitReversePermuteTable = 0;
	if(m_scratch)	free(m_scratch);	m_scratch = 0;
}

int PermuteServer::Permute(unsigned char *feature, unsigned char *tag, unsigned char *code)
{
	unsigned short flens = (unsigned short) m_length;

	// first we permute based on byte permutation table
	for(int i=0;i<2*m_length;i++)
	{
		unsigned short idx = m_bytePermutationTable[i];
		code[i] = (idx < flens)? feature[idx] : tag[idx-flens];
	}

	unsigned int *icode = (unsigned int *) code;

	for(int i=0;i<flens >> 1;i++)
		icode[i] = (icode[i] << m_permuteRotation) | (icode[i] >> m_reversePermuteRotation);

	for(int i=0;i<2*m_length;i++)
		code[i] = m_bitPermuteTable[code[i]];

	return 0;
}

int PermuteServer::Recover(unsigned char *code, unsigned char *feature, unsigned char *tag)
{
	unsigned short flens = (unsigned short) m_length;

		// fix bit permutation
	for(int i=0;i<2*m_length;i++)
		m_scratch[i] = m_bitReversePermuteTable[code[i]];

	// fix rotation
	unsigned int *icode = (unsigned int *) m_scratch;

	for(int i=0;i<flens >> 1;i++)
		icode[i] = (icode[i] >> m_permuteRotation) | (icode[i] << m_reversePermuteRotation);	
		
		
	// first we permute based on byte permutation table
	for(int i=0;i<2*m_length;i++)
	{
		unsigned short idx = m_bytePermutationTable[i];
		if(idx < flens) 
			feature[idx] = m_scratch[i];
		else
			tag[idx-flens] = m_scratch[i];
	}

	return 0;
}
