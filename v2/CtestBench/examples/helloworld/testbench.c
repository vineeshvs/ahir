#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
//
// the next two inclusions are
// to be used in the software version
//  
#include <iolib.h>
#include "prog.h"
//
//
void Exit(int sig)
{
	fprintf(stderr, "## Break! ##\n");
	exit(0);
}



typedef struct _FnArgs FnArgs;
struct _FnArgs
{
	int a;
	int* ret_val;
};

void *foo_(void* fargs)
{
   *(((FnArgs*)fargs)->ret_val) = foo(((FnArgs*)fargs)->a);
}

void *bar_(void* fargs)
{
   *(((FnArgs*)fargs)->ret_val) = bar(((FnArgs*)fargs)->a);
}

void *write_pipe_(void* a)
{
	write_uint32("inpipe",*((int*)a));
}

void *read_pipe_(void* a)
{
	*((int*)a) = read_uint32("outpipe");
}

int main(int argc, char* argv[])
{
	signal(SIGINT,  Exit);
  	signal(SIGTERM, Exit);

	int pipe_in, pipe_out, foo_ret, bar_ret;
	if(argc < 2)
	{
		fprintf(stderr, "specify seed\n");
		return(0);
	}
	int seed = atoi(argv[1]);
	pipe_in = seed;

	// in parallel, start foo and bar.
	pthread_t foo_t, bar_t, wpipe_t, rpipe_t;
	FnArgs foo_args, bar_args;

	foo_args.a = seed;
	foo_args.ret_val = &foo_ret;

	bar_args.a = seed;
	bar_args.ret_val = &bar_ret;

	pthread_create(&foo_t,NULL,&foo_,(void*)&foo_args);
	pthread_create(&bar_t,NULL,&bar_,(void*)&bar_args);
	pthread_create(&wpipe_t,NULL,&write_pipe_,(void*)&pipe_in);
	pthread_create(&rpipe_t,NULL,&read_pipe_,(void*)&pipe_out);


	pthread_join(foo_t,NULL);
	pthread_join(bar_t,NULL);
	pthread_join(wpipe_t,NULL);
	pthread_join(rpipe_t,NULL);

 	fprintf(stdout,"foo returns %d\n", foo_ret);
 	fprintf(stdout,"bar returns %d\n", bar_ret);
 	fprintf(stdout,"from outpipe, we read %d\n", pipe_out);
}
