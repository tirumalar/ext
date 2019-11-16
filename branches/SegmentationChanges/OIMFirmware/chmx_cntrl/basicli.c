#include <stdio.h>

/* A tiny BASIC interpreter */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stdio.h"
#include "setjmp.h"
#include "math.h"
#include "ctype.h"
#include "stdlib.h"
#include "string.h"
#include "basicli.h"





int  cli_get_char(void);

#define DELIMITER  1
#define VARIABLE  2
#define NUMBER    3
#define COMMAND   4
#define STRING	  5
#define QUOTE	  6
#define LABEL     7

#define PRINT 1
#define INPUT 2
#define IF    3
#define THEN  4
#define FOR   5
#define NEXT  6
#define TO    7
#define GOTO  8
#define EOL   9

#define FINISHED  10
#define GOSUB 11
#define RETURN 12
#define END 13
#define FUNC 14
#define FUNC_DEC 15

char *prog;  /* holds expression to be analyzed */
jmp_buf e_buf; /* hold environment for longjmp() */

#define iswhite(x) ((x)==' ' || (x)=='\t')
typedef struct
{ /* keyword lookup table */
	char command[20];
	char tok;
}commands;

commands table[] =
{ /* Commands must be entered lower case */
#if WITH_PRINT
		{"print", PRINT}, /* in this table. */
#endif    
#if WITH_INPUT
		{"input", INPUT},
#endif
		{"if", IF},
		{"then", THEN},
#if WITH_GOTO
		{"goto", GOTO},
#endif
		{"for", FOR},
		{"next", NEXT}, 
		{"to", TO},
#if WITH_GOSUB
	"gosub", GOSUB},
#endif
	{"return", RETURN},
	{"end", END},
	{"func",FUNC_DEC},
	{"", END}  /* mark end of table */
};


char token[MAX_TOKEN];
signed char token_type, tok;


/////// structures used for labels
struct label {
	char name[LAB_LEN];
	char *p;  /* points to place to go in source file*/
};
struct label label_table[NUM_LAB];



// this is a storage for passing a single string to a c linkage
char link_string[MAX_LINK_STRING];

IDX_TYPE func_table_count=0;
struct func func_table[NUM_FUNCS];

char local_func_arr[LOCAL_FUNC_NUM][LOCAL_FUN_SIZE];
IDX_TYPE local_func_cnt=0;

/////////////   structures for variables
struct var {
	char name[LAB_LEN];
	char type;
	int *value;
};

#define VAR_TYPE_INT_LOC 0
#define VAR_TYPE_INT_PTR 1

IDX_TYPE var_count=0;
struct var var_table[NUM_VARS];


///  stack related functions
char *find_label(char *s);
char *gpop(void);

typedef struct {
	int var; /* counter variable */
	int target;  /* target value */
	char *loc;
}  for_stack;

for_stack fstack[FOR_NEST]; /* stack for FOR/NEXT loop */
for_stack fpop(void);

char *gstack[SUB_NEST];	/* stack for gosub */

IDX_TYPE ftos;  /* index to top of FOR stack */
IDX_TYPE gtos;  /* index to top of GOSUB stack */


///// prototypes
int do_func(void);
int  get_token(void);
void assignment(void);

void print(void);
void scan_labels(void);
void find_eol(void);
void exec_goto(void);

void exec_if(void);
void exec_for(void);
void next(void);
void fpush(for_stack i);
void input(void);
void gosub(void);
int greturn(void);
void gpush(char *s);
void label_init(void);
void serror(int error);

void get_exp(int *result);

void putback(void);

void level2(int *result);
void level3(int *result);
void level4(int *result);
void level5(int *result);
void level6(int *result);
void primitive(int *result);
void unary(char o, int * r);
void arith(char o, int *r, int *h);

int call_func(char *fname, int *argv, int argc);

signed char look_up_func(char *s);
int parse_prog(char *p);
int look_up(char *s);
char isdelim(char c);
int get_next_label(char *s);



//#define IMPORT_PRINT_STR 1
////////////////////////////
#if IMPORT_PRINT_STR
void cli_print_str(char *p)
{
	printf("%s",p);
}
#endif

char i_print_str[14];

static void ui2a(unsigned int num, unsigned int base, int uc,char * bf)
    {
    int n=0;
    unsigned int d=1;
    while (num/d >= base)
        d*=base;        
    while (d!=0) {
        int dgt = num / d;
        num%= d;
        d/=base;
        if (n || dgt>0 || d==0) {
            *bf++ = dgt+(dgt<10 ? '0' : (uc ? 'A' : 'a')-10);
            ++n;
            }
        }
    *bf=0;
    }
    
static void i2a (int num, char * bf)
    {
    if (num<0) {
        num=-num;
        *bf++ = '-';
        }
    ui2a(num,10,0,bf);
    }
    
int cli_print_int(int i)
{
    int l;
    i2a(i,i_print_str);
    //sprintf(i_print_str,"%d",i);
    l=strlen(i_print_str);
    cli_print_str(i_print_str);
	return l;
}










int evaluate_func(char *fname, int *argv, int argc)
{
	int idx = look_up_func(fname);

	switch (func_table[idx].type)
	{
	case FUNC_EXT :return call_func(fname, argv,argc);
	case FUNC_INT :return parse_prog(func_table[idx].ptr);
	}
return 0;
}

void do_func_dec()
{
  char *p;
  int idx;
  
  get_token();
  idx = look_up_func(token);
  if (idx < 0)
  {
	  idx = func_table_count;
	  func_table_count++;
	  p = local_func_arr[local_func_cnt++];
	  func_table[idx].ptr = p;
  }
  else
	  p = func_table[idx].ptr;
  strcpy(func_table[idx].name, token);
  func_table[idx].argc = 0;
  func_table[idx].type = FUNC_INT;
  while (*prog)
  {
	  *p++=*prog++;  /*keep the spaces */


//code bellow was commented out at one point
	//  get_token();
	  //strcat(p,token);
  }
  *p=0;
}

int create_var (char *s)
{
	strcpy(var_table[var_count].name,s);
	var_table[var_count].type = VAR_TYPE_INT_LOC;
//	printf ("D>creating %s\n",s);
	return var_count++;

}

int find_var(char *s)
{
	int c;
	for (c=0; c < var_count;c++)
	{
		if (strcmp(s,var_table[c].name)==0)
			return c;
	}
	return -1;
}


/* Find the value of a variable. */
void set_var_idx_val_int(int idx, int value)
{
	switch(var_table[idx].type)
	{
	case VAR_TYPE_INT_LOC:var_table[idx].value= (int *)value;break;
	case VAR_TYPE_INT_PTR:*var_table[idx].value= value;break;
	}
}
void set_var_val_int(char *s,int val)
{
	char v;
	v=find_var(s);
	if (v<0){
		serror(4); /* not a variable */
		return;
	}
	set_var_idx_val_int(v, val);
	//var_table[v].value= (int *)val;
//	return variables[toupper(*token) - 'A'];
}

int  get_var_idx_val_int(int idx)
{
	switch(var_table[idx].type)
	{
	case VAR_TYPE_INT_LOC:return (int) var_table[idx].value;
	case VAR_TYPE_INT_PTR:return  *var_table[idx].value;
	}
	return 0;
}

/* Find the value of a variable. */
int find_var_val(char *s)
{
	char v;
	v=find_var(s);
	if (v<0){
		serror(4); /* not a variable */
		return 0;
	}
	return get_var_idx_val_int(v);
//	return variables[toupper(*token) - 'A'];
}


signed char look_up_func(char *s)
{
	int val;
	int args[10];
	val= call_func(s,args, -1);
//	printf("Looking for %s %d\n ",s,val);
	return (signed char) val;
}
#if 0
look_up_func(s)
char *s;
{
	register int i, j;
	char *p;

	/* convert to lowercase */
	p = s;
	while (*p){ *p = tolower(*p); p++; }

	/* see if token is in table */
	for (i = 0; func_table[i].name[0]; i++)
		{
		    if (strcmp(func_table[i].name, s)==0)  return i+1;
		}
	return 0; /* unknown command */
}

#endif



void cli_mutex_pend();
void cli_mutex_release();

int parse_prog(char *p)
{
     //  cli_mutex_pend();
      // while ( strchr("\r\n| ",p[strlen(p)-1])  && (*p!=0))
	//p[strlen(p)-1]=0;

	prog = p;
	scan_labels(); /* find the labels in the program */

    while (*prog)
	{
		token_type = get_token();
		/* check for assignment statement */
		if (token_type == VARIABLE) {
			putback(); /* return the var to the input stream */
			assignment(); /* must be assignment statement */
		}
		else /* is command */
			switch (tok) {
#if WITH_PRINT
			case PRINT:
				print();
				break;
#endif                
#if  WITH_GOTO
			case GOTO:
				exec_goto();
				break;
#endif                
			case IF:
				exec_if();
				break;
			case FOR:
				exec_for();
				break;
			case NEXT:
				next();
				break;
#if  WITH_INPUT
			case INPUT:
				input();
				break;
#endif
#if WITH_GOSUB
			case GOSUB:
				gosub();
				break;
#endif
            case RETURN:
        	//	cli_mutex_release();
				return greturn();
				break;
			case FUNC:
				 do_func();
				 break;
			case END:
			    //cli_mutex_release();
				 return 1;
			case FUNC_DEC:
				 do_func_dec();
		}
	}

    //cli_mutex_release();

return 0;
}


/* Assign a variable a value. */
void assignment(void)
{
	int var, value;
	char name[LAB_LEN];

	/* get the variable name */
	get_token();
	if (!isalpha(*token)) {
		serror(4);
		return;
	}

	var = find_var(token);
	strcpy(name,token);

	/* get the equals sign */
	get_token();
	if (*token != '=') {
		serror(3);
		return;
	}
	if (var <0)
		var=create_var(name);

	/* get the value to assign to var */
	get_exp(&value);

	/* assign the value */
	set_var_idx_val_int(var,value);
	//variables[var] = value;
}


#define MAX_ARGS 5
int do_func(void)
{
		int answer;
		int len = 0;
		int fn;
		int argc=0;
		int argv[MAX_ARGS];
		char fname[40];

		fn = look_up_func(token);
		strcpy(fname,token);


		// get the paranthesis
		if (func_table[fn].argc>0)
		{
			get_token();
			if (*token != '(') {
				serror(14);
				return 0;
			}
			do {
				get_token(); /* get next list item */
				if (tok == EOL || tok == FINISHED) break;
				if (*token==')') break;
				if (token_type == QUOTE) { /* is string */
	//				printf(token);
					strcpy(link_string,token);
					argv[argc++]= (int) link_string;
					len += strlen(token);
					get_token();
				}
				else { /* is expression */
					putback();
					get_exp(&answer);
					get_token();
					argv[argc++]= answer;
				}
			} while (*token == ';' || *token == ',');
		}
		// might not need get_token();
		if (argc!=func_table[fn].argc)
		{
			serror(14);
			return 0;
		}
		return evaluate_func(fname, argv,argc);
}


/* Execute a simple version of the BASIC PRINT statement */
#if WITH_PRINT
void print(void)
{
	int answer;
	int len = 0, spaces;
	char last_delim;

	do {
		get_token(); /* get next list item */
		if (tok == EOL || tok == FINISHED) break;
		if (token_type == QUOTE) { /* is string */
			cli_print_str(token);
			len += strlen(token);
			get_token();
		}
		else { /* is expression */
			putback();
			get_exp(&answer);
			get_token();
			len += cli_print_int(answer);
		}
		last_delim = *token;

		if (*token == ';') {
			/* compute number of spaces to move to next tab */
			spaces = 8 - (len % 8);
			len += spaces; /* add in the tabbing position */
			while (spaces) {
				cli_print_str(" ");
				spaces--;
			}
		}
		else if (*token == ',') /* do nothing */;

		else if (tok != EOL && tok != FINISHED) serror(0);
	} while (*token == ';' || *token == ',');

	if (tok == EOL || tok == FINISHED) {
		if (last_delim != ';' && last_delim != ',') cli_print_str("\n");
	}
	else serror(0); /* error is not , or ; */

}
#endif

/* Find all labels. */
void scan_labels(void)
{
	int addr;
	char *temp;

	label_init();  /* zero all labels */
	temp = prog;   /* save pointer to top of program */

	/* if the first token in the file is a label */
	get_token();
	if (token_type == NUMBER || (token_type == LABEL)) {
		strcpy(label_table[0].name, token);
		label_table[0].p = prog;
	}

	find_eol();
	do {
		get_token();
		if ((token_type == NUMBER) || (token_type == LABEL)) {
			addr = get_next_label(token);
			if (addr == -1 || addr == -2) {
				(addr == -1) ? serror(5) : serror(6);
			}
			strcpy(label_table[addr].name, token);
			label_table[addr].p = prog;  /* current point in program */
		}
		/* if not on a blank line, find next line */
		if (tok != EOL) find_eol();
	} while (tok != FINISHED);
	prog = temp;  /* restore to original */
}

/* Find the start of the next line. */
void find_eol(void)
{
	while (*prog != '\n'  && *prog != '\0'&& *prog != '|') ++prog;
	if (*prog) prog++;
}

/* Return index of next free position in label array.
A -1 is returned if the array is full.
A -2 is returned when duplicate label is found.
*/
int get_next_label(char *s)
{
	register int t;

	for (t = 0; t<NUM_LAB; ++t) {
		if (label_table[t].name[0] == 0) return t;
		if (!strcmp(label_table[t].name, s)) return -2; /* dup */
	}

	return -1;
}

/* Find location of given label.  A null is returned if
label is not found; otherwise a pointer to the position
of the label is returned.
*/
char *find_label(char *s)
{
	register int t;

	for (t = 0; t<NUM_LAB; ++t)
		if (!strcmp(label_table[t].name, s)) return label_table[t].p;
	return '\0'; /* error condition */
}

/* Execute a GOTO statement. */
void exec_goto(void)
{

	char *loc;

	get_token(); /* get label to go to */
	/* find the location of the label */
	loc = find_label(token);
	if (loc == '\0')
		serror(7); /* label not defined */

	else prog = loc;  /* start program running at that loc */
}

/* Initialize the array that holds the labels.
By convention, a null label name indicates that
array position is unused.
*/
void label_init(void)
{
	register int t;

	for (t = 0; t<NUM_LAB; ++t) label_table[t].name[0] = '\0';
}

/* Execute an IF statement. */
void exec_if(void)
{
	int x, y, cond;
	char op;

	get_exp(&x); /* get left expression */

	get_token(); /* get the operator */
	if (!strchr("=<>", *token)) {
		serror(0); /* not a legal operator */
		return;
	}
	op = *token;

	get_exp(&y); /* get right expression */

	/* determine the outcome */
	cond = 0;
	switch (op) {
	case '<':
		if (x<y) cond = 1;
		break;
	case '>':
		if (x>y) cond = 1;
		break;
	case '=':
		if (x == y) cond = 1;
		break;
	}
	if (cond) { /* is true so process target of IF */
		get_token();
		if (tok != THEN) {
			serror(8);
			return;
		}/* else program execution starts on next line */
	}
	else find_eol(); /* find start of next line */
}

/* Execute a FOR loop. */
void exec_for(void)
{
	for_stack i;
	int value;

	get_token(); /* read the control variable */
	if (!isalpha(*token)) {
		serror(4);
		return;
	}

	i.var = find_var(token);
	if (i.var <0)
		i.var=create_var(token);

	//toupper(*token) - 'A'; /* save its index */

	get_token(); /* read the equals sign */
	if (*token != '=') {
		serror(3);
		return;
	}

	get_exp(&value); /* get initial value */

	set_var_idx_val_int(i.var,value);
	//variables[i.var] = value;

	get_token();
	if (tok != TO) serror(9); /* read and discard the TO */

	get_exp(&i.target); /* get target value */

	/* if loop can execute at least once, push info on stack */
	if (value >= get_var_idx_val_int(i.var)) {
		i.loc = prog;
		fpush(i);
	}
	else  /* otherwise, skip loop code altogether */
		while (tok != NEXT) get_token();
}

/* Execute a NEXT statement. */
void next(void)
{
	for_stack i;

	i = fpop(); /* read the loop info */

	// variables[i.var]++; /* increment control variable */
	set_var_idx_val_int(i.var,get_var_idx_val_int(i.var)+1);

	if (get_var_idx_val_int(i.var)>i.target) return;  /* all done */
	fpush(i);  /* otherwise, restore the info */
	prog = i.loc;  /* loop */
}

/* Push function for the FOR stack. */
void fpush( for_stack i)

{
	if (ftos>FOR_NEST)
		serror(10);

	fstack[ftos] = i;
	ftos++;
}

for_stack fpop(void)
{
	ftos--;
	if (ftos<0) serror(11);
	return(fstack[ftos]);
}

#if WITH_INPUT
/* Execute a simple form of the BASIC INPUT command */
void input()
{
	char var;
	int i;

	get_token(); /* see if prompt string is present */
	if (token_type == QUOTE) {
		printf(token); /* if so, print it and check for comma */
		get_token();
		if (*token != ',') serror(1);
		get_token();
	}
	else printf("? "); /* otherwise, prompt with / */
	var = find_var(token);
	//toupper(*token) - 'A'; /* get the input var */

	scanf("%d", &i); /* read input */
	set_var_idx_val_int(var,i);
	//variables[var] = i; /* store it */
}

#endif

#if WITH_GOSUB
/* Execute a GOSUB command. */
void gosub(void)
{
	char *loc;

	get_token();
	/* find the label to call */
	loc = find_label(token);
	if (loc == '\0')
		serror(7); /* label not defined */
	else {
		gpush(prog); /* save place to return to */
		prog = loc;  /* start program running at that loc */
	}
}

#endif

/* Return from GOSUB. */
int greturn(void)
{
	int answer;
	get_exp(&answer);

	// prog = gpop();
	return (answer);
}

/* GOSUB stack push function. */
void gpush(char *s)
{
	gtos++;

	if (gtos == SUB_NEST) {
		serror(12);
		return;
	}

	gstack[gtos] = s;

}

/* GOSUB stack pop function. */
char *gpop(void)
{
	if (gtos == 0) {
		serror(13);
		return 0;
	}

	return(gstack[gtos--]);
}

/* Entry point into parser. */
void get_exp(int *result)
{
	get_token();
	if (!*token) {
		serror(2);
		return;
	}
	level2(result);
	putback(); /* return last token read to input stream */
}


/* display an error message */
void serror(int error)
{
#if WITH_LONG_ERROR
    static char *e[] = {
		"syntax error",
		"unbalanced parentheses",
		"no expression present",
		"equals sign expected",
		"not a variable",
		"Label table full",
		"duplicate label",
		"undefined label",
		"THEN expected",
		"TO expected",
		"too many nested FOR loops",
		"NEXT without FOR",
		"too many nested GOSUBs",
		"RETURN without GOSUB",
		"function parameters wrong"
	};
    cli_print_str(e[error]);
    cli_print_str("\n");
#else
    cli_print_str("ERR:");
    cli_print_int(error);
    cli_print_str("\n");
#endif
//	longjmp(e_buf, 1); /* return to save point */
}

/* Get a token. */
int  get_token(void)
{

	register char *temp;

	token_type = 0; tok = 0;
	temp = token;

	if (*prog == '\0') { /* end of file */
		*token = 0;
		tok = FINISHED;
		return(token_type = DELIMITER);
	}

	while (iswhite(*prog)) ++prog;  /* skip over white space */

	if (*prog == '\r') { /* crlf */
		++prog; 
		tok = EOL; *token = '\r';
		return (token_type = DELIMITER);
	}

	if (*prog == '|') { /*line separateor like crlf */
		*temp = *prog;
		prog++; /* advance to next position */
		temp++;
		*temp = 0;
		tok = EOL; *token = '|';
		return (token_type = DELIMITER);
	}

	if (strchr("+-*^/%=;(),><:&", *prog)){ /* delimiter */
		*temp = *prog;
		prog++; /* advance to next position */
		temp++;
		*temp = 0;
		return (token_type = DELIMITER);
	}

    
	if (*prog == '"') { /* quoted string */
		prog++;
		while (*prog != '"'&& *prog != '\r') *temp++ = *prog++;
		if (*prog == '\r') serror(1);
		prog++; *temp = 0;
		return(token_type = QUOTE);
	}

	if (isdigit(*prog)) { /* number */
		while (!isdelim(*prog)) *temp++ = *prog++;
		*temp = '\0';
		return(token_type = NUMBER);
	}

	if (isalpha(*prog)) { /* var or command */
		while (!isdelim(*prog)) *temp++ = *prog++;
		token_type = STRING;
	}

	*temp = '\0';

	/* see if a string is a command or a variable */
	if (token_type == STRING) {
		if (*prog == ':')
		{
			prog++;
			token_type = LABEL;
			return LABEL;
		}
		tok = look_up_func(token);
		if (tok>=0)
		   {
		    token_type = FUNC;
		    tok = FUNC;
		    return FUNC;
		   }
		tok = look_up(token); /* convert to internal rep */
		if (!tok) token_type = VARIABLE;
		else token_type = COMMAND; /* is a command */
	}
    
    if (token_type==0)
       {
        prog++; /* advance to next position */
	    temp++;
	    *temp = 0;
	    return (token_type = DELIMITER);
        }
	return token_type;
}



/* Return a token to input stream. */
void putback(void)
{

	char *t;

	t = token;
	for (; *t; t++) prog--;
}

/* Look up a a token's internal representation in the
token table.
*/
int look_up(char *s)
{
	register int i;
	char *p;

	/* convert to lowercase */
	p = s;
	while (*p){ *p = tolower(*p); p++; }

	/* see if token is in table */
	for (i = 0; *table[i].command; i++)
		if (!strcmp(table[i].command, s)) return table[i].tok;
	return 0; /* unknown command */
}

/* Return true if c is a delimiter. */
char isdelim(char c)
{
	if (strchr(" ;,+-<>/*%^=()|:&", c) || c == 9 || c == '\r' || c == 0)
		return 1;
	return 0;
}

/* Return 1 if c is space or tab. */
char iswhite2(char c)
{
	if (c == ' ' || c == '\t') return 1;
	else return 0;
}



/*  Add or subtract two terms. */
void level2(int *result)
{
	register char  op;
	int hold;

	level3(result);
	while ((op = *token) == '+' || op == '-') {
		get_token();
		level3(&hold);
		arith(op, result, &hold);
	}
}

/* Multiply or divide two factors. */
void level3(int *result)
{
	register char  op;
	int hold;

	level4(result);
	while ((op = *token) == '*' || op == '/' || op == '%' || op == '&') {
		get_token();
		level4(&hold);
		arith(op, result, &hold);
	}
}

/* Process integer exponent. */
void level4(int *result)
{
	int hold;

	level5(result);
	if (*token == '^') {
		get_token();
		level4(&hold);
		arith('^', result, &hold);
	}
}

/* Is a unary + or -. */
void level5(int *result)
{
	register char  op;

	op = 0;
	if ((token_type == DELIMITER) && (*token == '+' || *token == '-'))
	{
		op = *token;
		get_token();
	}
	level6(result);
	if (op)
		unary(op, result);
}

/* Process parenthesized expression. */
void level6(int * result)
{
	if ((*token == '(') && (token_type == DELIMITER)) {
		get_token();
		level2(result);
		if (*token != ')')
			serror(1);
		get_token();
	}
	else
		primitive(result);
}

/* Find value of number or variable. */
void primitive(int *result)
{

	switch (token_type) {
	case VARIABLE:
		*result = find_var_val(token);
		get_token();
		return;
	case NUMBER:
		{
			int val;
		if (token[1]=='X')
			val =strtoul (token,NULL, 0);
		else
			val =strtol (token,NULL, 0);
		*result = val;
				//atoi(token);
		get_token();
		return;
		}
	case FUNC:
		*result=do_func();
		get_token();
		return;
	default:
		serror(0);
	}
}

/* Perform the specified arithmetic. */
void arith(char o, int *r, int *h)
{
	register int t, ex;

	switch (o) {
	case '-':
		*r = *r - *h;
		break;
	case '+':
		*r = *r + *h;
		break;
	case '*':
		*r = *r * *h;
		break;
	case '/':
		*r = (*r) / (*h);
		break;
	case '%':
		t = (*r) / (*h);
		*r = *r - (t*(*h));
		break;
	case '&':
		*r = *r & *h;
		break;
	case '^':
		ex = *r;
		if (*h == 0) {
			*r = 1;
			break;
		}
		for (t = *h - 1; t>0; --t) *r = (*r) * ex;
		break;
	}
}

/* Reverse the sign. */
void unary(char o, int * r)
{
	if (o == '-') *r = -(*r);
}



#define MAX_ALL 2048
void save_all(char * all, int max_len)
{

 int c;
 char temp[80+LOCAL_FUN_SIZE];

 all[0]=0;
 for (c=0; c < func_table_count;c++)
	{
        if (func_table[c].type==FUNC_INT)
            {
            sprintf (temp,"func %s %s\r",func_table[c].name,func_table[c].ptr);
		    strcat(all,temp);
            }
	}
	for (c=0; c < var_count;c++)
	{
        if (var_table[c].type==VAR_TYPE_INT_LOC)
            {
            sprintf (temp,"%s=%d\r",var_table[c].name,(int)var_table[c].value);
		    strcat(all,temp);
            }
	}
}
void load_all(char *all)
{

 char *p=all;
 p = strtok(all,"\r");
 while (p)
 {
 parse_prog(p);
 p = strtok(NULL,"\r");
 }

}
int listv(void)
{
	int c;
    char temp[80];
	for (c=0; c < var_count;c++)
	{
		sprintf (temp,"%d %s %d %d \n",c,var_table[c].name,var_table[c].type,(int)var_table[c].value);
	    cli_print_str(temp);
	}

	return c;
}

int listf(void)
{
	int c;
    char temp[80 + LOCAL_FUN_SIZE];
	for (c=0;c<func_table_count;c++)
	{
		sprintf (temp,"%14s(%d) %s\n",func_table[c].name,func_table[c].argc,(func_table[c].type==FUNC_INT)?func_table[c].ptr:"LINKED");
		cli_print_str(temp);
	}
 return 0;
}

void menu(void)
{
	int c;
	int i;
	int argv;
    char temp[80];
while(1)
{
	for (c=0;c<func_table_count;c++)
	{
		sprintf (temp,"%2d %14s\n",c,func_table[c].name);
		cli_print_str(temp);
	}
 c = cli_get_char();
 if (c>='0' && c<(func_table_count+'0'))
	 {
	 c=c-'0';
	 sprintf (temp,"Exec %s\n",func_table[c].name);
     cli_print_str(temp);
	 evaluate_func(func_table[c].name, &argv,0);
	 }
 else
	 break;
}
 return 0;
}
int g_test=10;

void cli_printf( const char* format, ... ) 
{
    va_list args;
    char temp[MAX_PRINT];
    va_start( args, format );
    vsnprintf( temp,MAX_PRINT, format, args );
    va_end( args );
    cli_print_str(temp);
}



