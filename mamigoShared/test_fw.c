#include <stdio.h>

int verbose=1;
void setTestVerbosity(int v){ verbose=v;}
/*
Rounding error is error by 1.
*/

int ensure_byte_distance(const char* testName, unsigned char actual ,unsigned char expected ,int location,int dist)
{
	if(actual!=expected) {
		int diff=abs(actual-expected);
		if(diff < dist)
		{
			if(verbose)
			printf("%s ignoring distance error at %d, actual: %d, expected %d\n",testName,location,actual,expected);
		}
		else
		{
			fprintf(stderr,"%s failed at %d, actual: %d, expected %d\n",testName,location,actual,expected);
			return 0;
		}
	}
	return 1;
}

int ensure_byte_allowrounding(const char* testName, unsigned char actual ,unsigned char expected ,int location)
{
	if(actual!=expected) {
		char diff=actual-expected;
		if(diff==-1 || diff==1)
		{
			if(verbose)
			printf("%s ignoring rounding error at %d, actual: %d, expected %d\n",testName,location,actual,expected);
		}
		else
		{
			fprintf(stderr,"%s failed at %d, actual: %d, expected %d\n",testName,location,actual,expected);
			return 0;
		}
	}
	return 1;
}


int ensure_short_allowrounding(const char* testName, short actual ,short expected ,int location)
{
	if(actual!=expected) {
		short diff=actual-expected;
		if(diff==-1 || diff==1)
		{
			if(verbose)
			printf("%s ignoring rounding error at %d, actual: %d, expected %d\n",testName,location,actual,expected);
		}
		else
		{
			fprintf(stderr,"%s failed at %d, actual: %d, expected %d\n",testName,location,actual,expected);
			return 0;
		}
	}
	return 1;
}

int ensure_int_allowrounding(const char* testName, int actual ,int expected ,int location)
{
	if(actual!=expected) {
		int diff=actual-expected;
		if(diff==-1 || diff==1)
		{
			if(verbose)
			printf("%s ignoring rounding error at %d, actual: %d, expected %d\n",testName,location,actual,expected);
		}
		else
		{
			fprintf(stderr,"%s failed at %d, actual: %d, expected %d\n",testName,location,actual,expected);
			return 0;
		}
	}
	return 1;
}

int ensure_byte(const char* testName, unsigned char actual ,unsigned char expected ,int location)
{
	if(actual!=expected) {
		fprintf(stderr,"%s failed at %d, actual: %d, expected %d\n",testName,location,actual,expected);
		return 0;
	}
	return 1;
}
int ensure_int(const char* testName, unsigned int actual ,unsigned int expected ,int location)
{
	if(actual!=expected) {
		fprintf(stderr,"%s failed at %d, actual: %X, expected %X\n",testName,location,actual,expected);
		return 0;
	}
	return 1;
}

int ensure_short(const char* msg, unsigned short actual,unsigned short expected)
{
	if(actual!=expected) {
		fprintf(stderr,"%s::expected: %hu, actual: %hu\n",msg,expected,actual);
		return 0;
	}
	return 1;
}

int ensure_long(const char* msg, unsigned long expected,unsigned long actual)
{
	if(actual!=expected) {
		fprintf(stderr,"%s::expected:%lu actual:%lu \n",msg,expected,actual);
		return 0;
	}
	return 1;
}

int ensure_null(const char* msg, void* value)
{
	if(value) {
		fprintf(stderr,"%s::expected: 0, actual: %lu\n",msg,(unsigned long)value);
		return 0;
	}
	return 1;
}

int ensure_notnull(const char* msg, void* value)
{
	if(!value) {
		fprintf(stderr,"%s::expected: not null, actual: %lu\n",msg,(unsigned long)value);
		return 0;
	}
	return 1;
}
int ensure_results(const char* testName, unsigned char *actual, unsigned char* expected, short maxLen)
{
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_byte(testName,actual[i],expected[i],i))
		{
			fprintf(stderr,"\n%s FAILED\n",testName);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}

int ensure_results_int(const char* testName, int *actual, int *expected, short maxLen)
{
	int passed=1;
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_int(testName,(unsigned int)actual[i],(unsigned int)expected[i],i))
		{
			passed=0;
		}
	}
	if(passed) {if(verbose) printf("\n%s PASSED\n",testName);}
	else fprintf(stderr,"\n%s FAILED\n",testName);
	return passed;
}

//Madhav
int ensure_results_int_allowrounding(const char* testName, int *actual, int *expected, short maxLen)
{
	int passed=1;
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_int_allowrounding(testName,(unsigned int)actual[i],(unsigned int)expected[i],i))
		{
			passed=0;
		}
	}
	if(passed) {if(verbose) printf("\n%s PASSED\n",testName);}
	else fprintf(stderr,"\n%s FAILED\n",testName);
	return passed;
}



int ensure_results_short(const char* testName, short *actual, short *expected, short maxLen)
{
	int i=0;
	if(maxLen <=0)
	{
		fprintf(stderr,"\n%s FAILED count is -ve %d\n",testName, maxLen);
		return 0;
	}
	
	for(;i<maxLen;i++)
	{
		if(!ensure_short(testName,actual[i],expected[i]))
		{
			fprintf(stderr,"\n%s FAILED at location %d\n",testName, i);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}

int ensure_results_short_allowrounding(const char* testName, short *actual, short *expected, short maxLen)
{
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_short_allowrounding(testName,actual[i],expected[i],i))
		{
			fprintf(stderr,"\n%s FAILED at location %d\n",testName, i);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}
// check an array for a constant value
int ensure_results_const(const char* testName, unsigned int *actual, unsigned int expected ,short maxLen){
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_int(testName,actual[i],expected,i))
		{
			fprintf(stderr,"\n%s FAILED\n",testName);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}


// check an array for a constant value
int ensure_results_const_byte(const char* testName, unsigned char *actual, unsigned char expected ,short maxLen){
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_byte(testName,actual[i],expected,i))
		{
			fprintf(stderr,"\n%s FAILED\n",testName);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}

int ensure_results_byte_distance(const char* testName, unsigned char *actual, unsigned char *expected ,short maxLen,int dist){
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_byte_distance(testName,actual[i],expected[i],i,dist))
		{
			fprintf(stderr,"\n%s FAILED\n",testName);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}

/*
Rouding error is error by 1.
*/
int ensure_results_allowrounding(const char* testName, unsigned char *actual, unsigned char* expected, short maxLen)
{
	int i=0;
	for(;i<maxLen;i++)
	{
		if(!ensure_byte_allowrounding(testName,actual[i],expected[i],i))
		{
			fprintf(stderr,"\n%s FAILED\n",testName);
			return 0;
		}
	}
	if(verbose) printf("\n%s PASSED\n",testName);
	return 1;
}
