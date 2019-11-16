#define NUM_LAB 5   // Number of labels not used
#define LAB_LEN 10    // char length of label
#define FOR_NEST 5
#define SUB_NEST 5
#define FUNC_NAME_LEN 20  //char length of a function name
#define NUM_FUNCS    50    // number of allowable functions
#define MAX_TOKEN  200   // char length of a single token
#define NUM_VARS 10     // total number of variable (local + linked)
#define MAX_LINK_STRING 80  // this is the maximum string length that will passed to a C function

#define LOCAL_FUNC_NUM 30
#define LOCAL_FUN_SIZE 255

#define MAX_PRINT 255  // longest thing you can print

/////// structures used for function calls
#define FUNC_EXT 0
#define FUNC_INT 1


struct func {
	char name[FUNC_NAME_LEN];
	char type;
	char argc;
	char *ptr;
};



////////////////////////// BLINKING functions
#define BLINK_VAR_INT(v)  strcpy(var_table[var_count].name,#v);var_table[var_count].type = VAR_TYPE_INT_PTR;var_table[var_count++].value = &v;


#define BLINK_START int call_func(char *fname, int *argv, int argc) {int inarg=0; int fcnt=0; int ret_val=-1;
#define INT_BLINK(F,c)  \
		if (argc==-2) \
			{\
			strcpy(func_table[fcnt].name,#F);\
			func_table[fcnt].argc=c ;\
			func_table[fcnt].type=FUNC_EXT;\
			}\
			if (strcmp(fname,#F)==0) \
				{\
				if (argc==-1) \
					return fcnt;\
				else  \
                { \
                    ret_val= F( \

#define VOID_BLINK(F,c)  \
		if (argc==-2) \
			{\
			strcpy(func_table[fcnt].name,#F);\
			func_table[fcnt].argc=c ;\
			func_table[fcnt].type=FUNC_EXT;\
			}\
			if (strcmp(fname,#F)==0) \
				{\
				if (argc==-1) \
					return fcnt;\
				else  \
                    {\
                        ret_val=0;\
                        F( 


#define BLINK_DONE          );\
                        return ret_val;\
                    };\
                  }\
                fcnt++;


#define BLINK_END if (argc==-2) func_table_count=fcnt;\
					else while (fcnt!=func_table_count){if(strcmp(func_table[fcnt].name,fname)==0) return fcnt;fcnt++;}\
					return -1;}

#define BLINKINT     argv[inarg++]
#define BLINKSTRING  ((char*)argv[inarg++])

#define IDX_TYPE  unsigned char
extern IDX_TYPE func_table_count;

extern struct func func_table[NUM_FUNCS];

extern char local_func_arr[LOCAL_FUNC_NUM][LOCAL_FUN_SIZE];

extern IDX_TYPE ftos;  /* index to top of FOR stack */
extern IDX_TYPE gtos;  /* index to top of GOSUB stack */



                
int listv(void);
int listf(void);       


#define WITH_INPUT          0
#define WITH_LONG_ERROR     1
#define WITH_GOTO           1
#define WITH_PRINT          1
#define WITH_LISTV          1
#define WITH_LISTF          1




void cli_printf( const char* format, ... );

void cli_print_str(char *p);
int cli_print_int(int i);
int basicli_init(void);
void menu(void);
