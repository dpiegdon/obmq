
#include "obmq.h"

#include <stdio.h>
#include <unistd.h>

static unsigned time = 0;

void setchan(char value)
{
#if 0
	printf("%d %c\n", time, (value)?'1':'0');
#else
	printf("%c\n", (value)?'1':'0');
#endif
	fflush(stdout);
}

int main()
{
	OneBitMessageQueue m;
	obmq_init(&m, setchan, 0, 0, 3, 8);

#if 1
	obmq_queuemessage(&m, 'd');
	obmq_queuemessage(&m, 'a');
	obmq_queuemessage(&m, 'v');
	obmq_queuemessage(&m, 'i');
	obmq_queuemessage(&m, 'd');
	obmq_queuemessage(&m, '!');
#endif

	while(1) {
		// main event loop
		// [...] do some stuff

		obmq_trigger(&m);

		usleep(50000);
		++time;
	}
}

