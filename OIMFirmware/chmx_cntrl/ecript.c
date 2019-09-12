#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Enable ECB, CTR and CBC mode. Note this can be done before including aes.h or at compile-time.
// E.g. with GCC by using the -D flag: gcc -c aes.c -DCBC=0 -DCTR=1 -DECB=1
#define CBC 1
#define CTR 1
#define ECB 1

#include "aes.h"
struct AES_ctx ctx;


uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
uint8_t iv[]  = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

int Decrypt(char *data_in, int len);
int Encrypt(char *data_in, int len);

void Encrypt_Init()
    {
    static int once=0;
    if (once==0)
	{
	AES_init_ctx_iv(&ctx, key,iv);
	//once =1;
	}

    }
int Encrypt(char *data_in, int len)
{
    Encrypt_Init();
   while ((len%16)!=0)
       {
       data_in[len]=0;
       len++;
       }

   AES_CBC_encrypt_buffer(&ctx, data_in, len);
}

int Decrypt(char *data_in, int len)
    {
    Encrypt_Init();
    AES_CBC_decrypt_buffer(&ctx, data_in, len);

    //AES_CBC_decrypt_buffer(&ctx, data_in, len);
    }
