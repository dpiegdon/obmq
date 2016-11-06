
#include "obmq.h"

#include <stdio.h>
#include <unistd.h>

static unsigned time = 0;

/*
#define PLOTTABLE_OUTPUT
#define DEBUG_QUEUE
*/

void setchan(void * data, char value)
{
#ifdef PLOTTABLE_OUTPUT
	/* for gnuplot testing: */
	static char v;
	printf("%d %d\n", time*2, v);
	v = value;
	printf("%d %d\n", time*2+1, v);
#else
	printf("%c\n", (value)?'1':'0');
#endif
	fflush(stdout);
}

int main()
{
	OneBitMessageQueue m;
	char message[] = {1, 2, 3, 4, 5, 6};
	char *p = message;

	obmq_init(&m, setchan, NULL, 0, 0, 0, 8, 0);
#ifdef PLOTTABLE_OUTPUT
	fprintf(stderr, "you can interrupt after a second. then plot via:\n"
			"  gnuplot> plot 'outfile' using 1:2 with linespoints\n");
#else
	sleep(1);
#endif

	while(1) {
		if(*p && obmq_messages_free(&m)) {
#ifdef debug_queue
			fprintf(stderr, "free %u queued %u.\n", obmq_messages_free(&m), obmq_messages_queued(&m));
#endif
			obmq_queuemessage(&m, *p);
#ifdef debug_queue
			fprintf(stderr, "queued message %u.\n", (unsigned)*p);
#endif
			++p;
		}

		obmq_trigger(&m);

#ifndef PLOTTABLE_OUTPUT
		usleep(50000);
#endif
		++time;
	}
}

