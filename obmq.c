#include <stdio.h>
#define printf(...)

#include "obmq.h"

void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(char), unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength)
{
	m->mSetChan = set_channel_value;
	m->mSlowdown = slowdown;
	m->mRepeatMsg = repeat_message;
	m->mInterMessageClocks = inter_message_clocks;
	m->mBitLength = 2*bitlength;

	m->started = 0;
	m->mCurrentSlowdown = 0;
	m->mNextMessage = 0;

	m->mCurrentMsgRepeat = 0;
	m->mCurrentBit = 0;
	m->mCurrentInBit = 0;
	m->mCurrentValue = 2;
}

#define ISINTERMESSAGE(m) (8 <= (m)->mCurrentBit)
#define ISINTERBIT(m) ((m)->mBitLength <= (m)->mCurrentInBit)
#define ISINTERBIT_END(m) ((m)->mBitLength+1 <= (m)->mCurrentInBit)
#define NEXTBIT(m) { (m)->mCurrentBit++; (m)->mCurrentInBit = 0; }
#define NEXTINTERBIT(m) { (m)->mCurrentInBit++; }

char obmq_get_next_bitstate(OneBitMessageQueue * m)
{
	char newValue = 2;

	if(m->started <= 2*(m->mInterMessageClocks+1)) {
		if(obmq_messages_queued(m)) {
			newValue = m->started & 1;
			m->started++;
		};
		return newValue;
	}

	// do we need to get a new message?
	// that is so, if the inter-message clocks have been played
	if(ISINTERMESSAGE(m) && m->mInterMessageClocks*2 <= m->mCurrentInBit)
	{
printf("new message\n");
		// can we no longer repeat the previous one?
		if(m->mCurrentMsgRepeat >= m->mRepeatMsg) {
			char newMsg;
			m->mCurrentMsgRepeat = 0;
			newMsg = m->mCurrentMessage + 1;
			if(newMsg >= OBMQ_MESSAGE_QUEUE)
				newMsg = 0;
			if(newMsg != m->mNextMessage) {
				m->mCurrentMessage = newMsg;
			} else {
				// idle. just repeat previous message again.
			}
		} else {
			++(m->mCurrentMsgRepeat);
		}
		m->mCurrentBit = 0;
		m->mCurrentInBit = 0;
	};

printf("bit %d inbit %d\n", (int)m->mCurrentBit, (int)m->mCurrentInBit);
	if(ISINTERMESSAGE(m)) {
printf("  inter-message\n");
		newValue = m->mCurrentInBit & 1;
		++(m->mCurrentInBit);
	} else if(m->mCurrentMessage != m->mNextMessage) { /* valid message */
		if(ISINTERBIT(m)) {
printf("  end-bits\n");
			newValue = m->mCurrentInBit & 1;
			if(ISINTERBIT_END(m)) {
				m->mCurrentInBit = 0;
				++(m->mCurrentBit);
			} else {
				++(m->mCurrentInBit);
			}
		} else {
printf("  in-bit\n");
			newValue = m->mMessages[m->mCurrentMessage] >> (7-m->mCurrentBit) & 1;
			++(m->mCurrentInBit);
		}
	} else {
		// no message was ever queued
	}

	return newValue;
}

void obmq_trigger(OneBitMessageQueue * m)
{
	if(0 == m->mSetChan)
		return;

	if(m->mCurrentSlowdown) {
		--(m->mCurrentSlowdown);
		return;
	};
	m->mCurrentSlowdown = m->mSlowdown;

	char bit = obmq_get_next_bitstate(m);
	if(m->mCurrentValue != bit && bit >= 0 && bit <= 1) {
		m->mCurrentValue = bit;
		m->mSetChan(bit);
	}
}

void obmq_queuemessage(OneBitMessageQueue * m, char message)
{
	if(0 == obmq_messages_free(m))
		return;

	m->mMessages[m->mNextMessage] = message;
	++(m->mNextMessage);
	if(m->mNextMessage >= OBMQ_MESSAGE_QUEUE)
		m->mNextMessage = 0;
}

int obmq_messages_queued(OneBitMessageQueue * m)
{
	int ret;

	ret = m->mNextMessage - m->mCurrentMessage;
	if(ret < 0)
		ret = -ret;

	return ret;
}

int obmq_messages_free(OneBitMessageQueue * m)
{
	return OBMQ_MESSAGE_QUEUE - obmq_messages_queued(m);
}

