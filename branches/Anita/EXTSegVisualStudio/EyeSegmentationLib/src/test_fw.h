#ifndef TEST_FW_H_
#define TEST_FW_H_
int ensure_byte_distance(const char* testName, unsigned char actual ,unsigned char expected ,int location,int dist);
int ensure_byte(const char* testName, unsigned char actual ,unsigned char expected ,int location);
int ensure_int(const char* testName, unsigned int actual ,unsigned int expected ,int location);
int ensure_int_allowrounding(const char* testName, int actual ,int expected ,int location);
int ensure_short(const char* msg, unsigned short expected,unsigned short actual);
int ensure_long(const char* msg, unsigned long expected,unsigned long actual);
int ensure_null(const char* msg, void* value);
int ensure_notnull(const char* msg, void* value);
int ensure_results(const char* testName, unsigned char *actual, unsigned char* expected, short maxLen);
int ensure_results_int(const char* testName, int *actual, int *expected, short maxLen);

int ensure_results_short(const char* testName, short *actual, short *expected, short maxLen);
int ensure_results_short_allowrounding(const char* testName, short *actual, short *expected, short maxLen);
int ensure_results_allowrounding(const char* testName, unsigned char *actual, unsigned char* expected, short maxLen);
int ensure_results_int_allowrounding(const char* testName, int *actual, int *expected, short maxLen);

int ensure_results_const(const char* testName, unsigned int *actual, unsigned int expected ,short maxLen);
int ensure_results_const_byte(const char* testName, unsigned char *actual, unsigned char expected ,short maxLen);
int ensure_results_byte_distance(const char* testName, unsigned char *actual, unsigned char *expected ,short maxLen,int dist);
void setTestVerbosity(int v);

#endif //TEST_FW
