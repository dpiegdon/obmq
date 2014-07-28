
// OneBit Message Queue

#ifndef _OBMQ_H_
# define _OBMQ_H_

# define OBMQ_MESSAGE_QUEUE 16


// this class is supposed to be opaque!
// do not change anything inside it!
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

// Initialise a queue
//	set_channel_value		callback to set bit-value of output
//	slowdown			a clock divider for the event-loop triggers
//	repeat_message			how often a message will be repeated after sending it
//					(i.e. if 1, a message will be sent twice on the channel)
//	bitlength			length of a bit in clocks
void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(char), unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength);

// Trigger in EventLoop
void obmq_trigger(OneBitMessageQueue * m);

// Queue a new message
void obmq_queuemessage(OneBitMessageQueue * m, char message);

// Number of currently queued messages
int obmq_messages_queued(OneBitMessageQueue * m);

// Number of free messages in queue
int obmq_messages_free(OneBitMessageQueue * m);

#endif // _OBMQ_H_

