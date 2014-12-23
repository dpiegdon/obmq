
#include "obmq.h"

#include <stdio.h>
#include <unistd.h>

static unsigned time = 0;

void setchan(void * data, char value)
{
#if 0
	// for gnuplot testing:
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
	sleep(1);

	while(1) {
		if(*p && obmq_messages_free(&m)) {
			fprintf(stderr, "free %u queued %u.\n", obmq_messages_free(&m), obmq_messages_queued(&m));
			obmq_queuemessage(&m, *p);
			fprintf(stderr, "queued message %u.\n", (unsigned)*p);
			++p;
		}

		obmq_trigger(&m);

		usleep(50000);
		++time;
	}
}

