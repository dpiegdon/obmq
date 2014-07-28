
// OneBit Message Queue

#ifndef _OBMQ_H_
# define _OBMQ_H_

# define OBMQ_MESSAGE_QUEUE 64

typedef struct {
	// parameters
	void(*mSetChan)(char);
	char mSlowdown;
	char mRepeatMsg;
	char mInterMessageClocks;
	char mBitLength;

	// state
	char started;

	char mCurrentSlowdown;

	char mMessages[OBMQ_MESSAGE_QUEUE];
	char mCurrentMessage;
	char mNextMessage;

	char mCurrentMsgRepeat;

	char mCurrentBit;	// 0..7 are for a message; 8 is inter-message

	char mCurrentInBit;	// in Msg: 0..BIT_LEN-1 are in bits;
				//         BIT_LEN..BIT_LEN+1 is inter-bit
				// InterMessage: 0..INTERMESSAGE_CLOCKS*2
	char mCurrentValue;
} OneBitMessageQueue;


void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(char), unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength);
void obmq_trigger(OneBitMessageQueue * m);
void obmq_queuemessage(OneBitMessageQueue * m, char message);
int obmq_messages_queued(OneBitMessageQueue * m);
int obmq_messages_free(OneBitMessageQueue * m);

#endif // _OBMQ_H_

