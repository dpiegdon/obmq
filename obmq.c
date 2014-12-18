
#ifdef __cplusplus
extern "C" {
#endif

#include "obmq.h"

static inline unsigned obmq_buffer_next_index(unsigned index)
{
	return (index+1 < OBMQ_MESSAGE_QUEUE) ? index+1 : 0;
}

static inline char obmq_peekmessage(OneBitMessageQueue * m)
{
	return m->mMessages[m->mCurrentMessage];
}

static inline void obmq_pokemessage(OneBitMessageQueue *m, char message)
{
	m->mMessages[m->mNextMessage] = message;
}

static inline void obmq_freemessage(OneBitMessageQueue *m)
{
	unsigned nextMessage = obmq_buffer_next_index(m->mCurrentMessage);
	if(m->mNextMessage != nextMessage)
		m->mCurrentMessage = nextMessage;
}


void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(void*, char), void * set_channel_data, unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength)
{
	m->mSetChan = set_channel_value;
	m->mSetChanData = set_channel_data;
	m->mSlowdown = slowdown;
	m->mRepeatMsg = repeat_message;
	m->mInterMessageClocks = inter_message_clocks;
	m->mBitLength = 2*bitlength;

	m->started = 0;
	m->mCurrentSlowdown = 0;
	m->mCurrentMessage = 0;
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
		// can we no longer repeat the previous one?
		if(m->mCurrentMsgRepeat >= m->mRepeatMsg) {
			m->mCurrentMsgRepeat = 0;
			obmq_freemessage(m);
		} else {
			++(m->mCurrentMsgRepeat);
		}
		m->mCurrentBit = 0;
		m->mCurrentInBit = 0;
	};

	if(ISINTERMESSAGE(m)) {
		newValue = m->mCurrentInBit & 1;
		++(m->mCurrentInBit);
	} else if(m->mCurrentMessage != m->mNextMessage) { /* valid message */
		if(ISINTERBIT(m)) {
			newValue = m->mCurrentInBit & 1;
			if(ISINTERBIT_END(m)) {
				m->mCurrentInBit = 0;
				++(m->mCurrentBit);
			} else {
				++(m->mCurrentInBit);
			}
		} else {
			newValue = obmq_peekmessage(m) >> (7-m->mCurrentBit) & 1;
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
	if(m->mCurrentValue != bit && (0 == bit || 1 == bit)) {
		m->mCurrentValue = bit;
		m->mSetChan(m->mSetChanData, bit);
	}
}

void obmq_queuemessage(OneBitMessageQueue * m, char message)
{
	if(0 == obmq_messages_free(m))
		return;

	obmq_pokemessage(m, message);
	m->mNextMessage = obmq_buffer_next_index(m->mNextMessage);
}

unsigned obmq_messages_queued(OneBitMessageQueue * m)
{
	return (OBMQ_MESSAGE_QUEUE + m->mNextMessage - m->mCurrentMessage) % OBMQ_MESSAGE_QUEUE;
}

unsigned obmq_messages_free(OneBitMessageQueue * m)
{
	return OBMQ_MESSAGE_QUEUE - obmq_messages_queued(m) - 1;
}


#ifdef __cplusplus
}
#endif

