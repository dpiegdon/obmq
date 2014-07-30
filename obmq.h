
#ifdef __cplusplus
extern "C" {
#endif

// OneBit Message Queue

#ifndef _OBMQ_H_
# define _OBMQ_H_

# define OBMQ_MESSAGE_QUEUE 64



// this class is supposed to be opaque!
// do not change anything inside it!
typedef struct {
	// parameters
	void(*mSetChan)(void*, char);
	void *mSetChanData;
	unsigned mSlowdown;
	char mRepeatMsg;
	char mInterMessageClocks;
	char mBitLength;
	// state
	char started;
	char mCurrentSlowdown;
	char mMessages[OBMQ_MESSAGE_QUEUE];
	unsigned mCurrentMessage;
	unsigned mNextMessage;
	char mCurrentMsgRepeat;
	char mCurrentBit;	// 0..7 are for a message; 8 is inter-message
	char mCurrentInBit;	// in Msg: 0..BIT_LEN-1 are in bits;
				//         BIT_LEN..BIT_LEN+1 is inter-bit
				// InterMessage: 0..INTERMESSAGE_CLOCKS*2
	char mCurrentValue;
} OneBitMessageQueue;

// Initialise a queue
//	set_channel_value		callback to set bit-value of output
//	set_channel_data		void* that will be passed to each call of set_channel_value
//	slowdown			a clock divider for the event-loop triggers
//	repeat_message			how often a message will be repeated after sending it
//					(i.e. if 1, a message will be sent twice on the channel)
//	inter_message_clocks		how many clocks between a message?
//					(effectively is one more due to inter-bit-clock)
//	bitlength			length of a bit in clocks
void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(void*, char), void * set_channel_data, unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength);

// Trigger in EventLoop
void obmq_trigger(OneBitMessageQueue * m);

// Queue a new message
void obmq_queuemessage(OneBitMessageQueue * m, char message);

// Number of currently queued messages
unsigned obmq_messages_queued(OneBitMessageQueue * m);

// Number of free messages in queue
unsigned obmq_messages_free(OneBitMessageQueue * m);

#endif // _OBMQ_H_

#ifdef __cplusplus
}
#endif

