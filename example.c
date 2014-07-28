
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

	obmq_queuemessage(&m, 'd');
	obmq_queuemessage(&m, 'e');
	obmq_queuemessage(&m, 'b');
	obmq_queuemessage(&m, 'u');
	obmq_queuemessage(&m, 'g');
	obmq_queuemessage(&m, '!');

	while(1) {
		// main event loop
		// [...] do some stuff

		obmq_trigger(&m);

		usleep(100000);
		++time;
	}
}

