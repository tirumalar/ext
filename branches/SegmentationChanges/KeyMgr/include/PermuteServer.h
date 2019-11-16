#pragma once

class  PermuteServer
{
public:
	PermuteServer(int length, unsigned int permutationKey=0);
	virtual ~PermuteServer(void);
	int Permute(unsigned char *feature, unsigned char *tag, unsigned char *code);
	int Recover(unsigned char *code, unsigned char *feature, unsigned char *tag);
	int GetPermutationKey(){ return m_permutationKey;}
private:
	int m_permuteRotation;
	int m_reversePermuteRotation;
	unsigned short *m_bytePermutationTable;
	unsigned char *m_bitPermuteTable;
	unsigned char *m_bitReversePermuteTable;
	unsigned int m_permutationKey;
	int m_length;
	unsigned char *m_scratch;
};
