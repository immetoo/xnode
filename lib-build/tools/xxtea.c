#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

void btea(uint32_t *v, int n, uint32_t const key[4]) {
  uint32_t y, z, sum;
  unsigned p, rounds, e;
  if (n > 1) {          /* Coding Part */
    rounds = 6 + 52/n;
    sum = 0;
    z = v[n-1];
    do {
      sum += DELTA;
      e = (sum >> 2) & 3;
      for (p=0; p<n-1; p++) {
	y = v[p+1]; 
	z = v[p] += MX;
      }
      y = v[0];
      z = v[n-1] += MX;
    } while (--rounds);
  } else if (n < -1) {  /* Decoding Part */
    n = -n;
    rounds = 6 + 52/n;
    sum = rounds*DELTA;
    y = v[0];
    do {
      e = (sum >> 2) & 3;
      for (p=n-1; p>0; p--) {
	z = v[p-1];
	y = v[p] -= MX;
      }
      z = v[n-1];
      y = v[0] -= MX;
      sum -= DELTA;
    } while (--rounds);
  }
}

void convToQU32(uint32_t *v/*, int n*/,int *data,int dataOffset) {
	for (int i=0;i<4;i++) {
		int idx = dataOffset + i*4;
		//uint32_t value = (data[idx]) + (data[idx+1] << 8) + (data[idx+2] << 16) + (data[idx+3] << 24);
		uint32_t value = (data[idx+3]) + (data[idx+2] << 8) + (data[idx+1] << 16) + (data[idx+0] << 24);
		printf("convToQU32 %08x \n", value);
		v[i] = value;
	}
}

#define XXTEA_NUM_ROUNDS 32
void xxteaDecrypt(unsigned long v[2],unsigned long net_key[4]) 
{
	unsigned int i;
	uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*XXTEA_NUM_ROUNDS;
	for (i=0; i < XXTEA_NUM_ROUNDS; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + net_key[(sum>>11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + net_key[sum & 3]);
	}
	v[0]=v0; v[1]=v1;
}

int main (int argc, char *argv[] ) {
	// Arguments parsing
	if ( argc != 4 ) {
		printf( "usage: %s encrypt|decrypt key message\n", argv[0] ); // prog name
		printf( "note: parameters are pure hex and key should be 32 chars. \n");
		return 1;
	}
	int operation = 0;
	if (strcmp("encrypt", argv[1]) == 0) {
		operation = 1;
	}
	if (strcmp("decrypt", argv[1]) == 0) {
		operation = -1;	
	}
	if (operation == 0) {
		printf( "Could not parse first argument.\n");
		return 1;
	}
	printf( "operation: %d.\n",operation);
	char* keyBuffer = argv[2];
	int keyBufferLength = strlen(keyBuffer);
	if (keyBufferLength != 32) {
		printf( "key should by 32 chars.\n");
		return 1;
	}
	char* messageBuffer = argv[3];
	int messageBufferLength = strlen(messageBuffer);
	if (messageBufferLength < 16) {
		printf( "message should be minimal 16 chars.\n");
		return 1;
	}
	// Done arguments
	
	// Convert data
	int keyData[16];
	int keyDataIndex = 0;
	for (int i=0;i<keyBufferLength/2;i++) {
		int value;
		sscanf(keyBuffer+2*i,"%02x",&value);
		keyData[keyDataIndex] = value;
		keyDataIndex++;
	}
	printf( "Key data size %d\n", keyDataIndex );
	for (int i=0;i<keyDataIndex;i++) {
		printf( "%02x", keyData[i] );
	}
	printf("\n");
	
	int messageData[512];
	int messageDataIndex = 0;
	for (int i=0;i<512;i++) {
		messageData[i] = 0;
	}
	for (int i=0;i<messageBufferLength/2;i++) {
		int value;
		sscanf(messageBuffer+2*i,"%02x",&value);
		messageData[messageDataIndex] = value;
		messageDataIndex++;
	}
	printf( "Message data size %d\n", messageDataIndex );
	for (int i=0;i<messageDataIndex;i++) {
			printf( "%02x", messageData[i] );
		}
		printf("\n");
	
	uint32_t key[4];
	uint32_t data[4];
	
	convToQU32(key,keyData,0);
	convToQU32(data,messageData,0);

	printf("OUT-ppre: ");
	for (int i=0;i<4;i++) {
		printf("%08x", data[i]);
	}
	printf("\n");
	
	operation = operation* (messageDataIndex/4);
	printf( "word length: %d\n",operation);
	
	//btea(data,operation,key);
	xxteaDecrypt(data,key);

	printf("OUT-post: ");
	for (int i=0;i<4;i++) {
		printf("%08x", data[i]);
	}
	printf("\n");
	
	printf("OUT-AS: ");
	for (int i=0;i<4;i++) {
		//printf("%02x ", data[i] & 255);
		//printf("%02x ", data[i] >> 8 & 255);
		//printf("%02x ", data[i] >> 16 & 255);
		//printf("%02x ", data[i] >> 24 & 255);
		
		printf("%c ", data[i] & 255);
		printf("%c ", data[i] >> 8 & 255);
		printf("%c ", data[i] >> 16 & 255);
		printf("%c ", data[i] >> 24 & 255);
	}
	printf("\n");
	return 0;
}
