#ifndef __USERPROG__
#define __USERPROG__

// typedef struct missile_code {
//   int numprocs;
//   char really_important_char;
// } missile_code;

typedef struct cbuffer {
	char store[BUFFERSIZE];
	int start;
	int end;
	int count; //used for sync between muiltiple processes
} cbuffer;

#define PRODUCER_FILENAME_TO_RUN "producer.dlx.obj"
#define CONSUMER_FILENAME_TO_RUN "consumer.dlx.obj"

#endif