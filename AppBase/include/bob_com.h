// Bob api

int Bob_GetHW_Version();
int Bob_GETSW_Version();


int  BobReadExternalPower();
int  BobReadPowerSource();

int  BobGetSounderIn();
int  BobGetLedRIn();
int  BobGetLedGIn();
int  BobGetTamperIn();
int  BobGetReed1();
int  BobGetReed2();

int  BobSetRelay1(int val);
int  BobSetRelay2(int val);
int  BobSetSounderOUT(int val);
int  BobSetLedROUT(int val);
int  BobSetLedGOUT(int val);
void  BobSetTamperOUT(int val);
int BobGetAllInputs(void);

// returns NULL len =0 if no data was received
int Bob_Read_last_Wiegand(char *data,  int *len);
int Bob_Send_Weigand(char *data, int len);
int Bob_Set_WeigandMode(int mode);
#define BOB_MODE_WEIGAND 1
#define BOB_MODE_F2F     2


int Bob_Read_UART1(char *data, int len , int timeout);
int Bob_Send_UART1(char *data, int len );

int Bob_Read_UART2(char *data, int *len , int timeout);
int Bob_Send_UART2(char *data, int *len );

int Bob_Set_UART1_Mode(int baud, int mode, int parity);
int Bob_Set_UART2_Mode(int baud, int mode, int parity);

#define BOB_BAUD_300     1
#define BOB_BAUD_1200    2
#define BOB_BAUD_2400    3
#define BOB_BAUD_9600    4
#define BOB_BAUD_19200   5

#define BOB_UART_MODE_232 1
#define BOB_UART_MODE_485 2

#define BOB_UART_PARITY_NONE 0
#define BOB_UART_PARITY_EVEN 0
#define BOB_UART_PARITY_ODD  1



int Bob_Read_Local_Tamperl();
int Bob_Clear_Local_Tamper();


int BobInitComs();
int BobCloseComs();

// set call back function which gets called if an input changes
// ie led in tamper in sound in
int BobSetInputCB(void (*cb)());

// gets called if a weigand input is received
int BobSetWeigandCB(void (*cb)());

