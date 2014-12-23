
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
	char mInfiniteRepeat;
	// state
	char mStarted;			// counter for start-flashes
	char mCurrentSlowdown;		// counter for trigger slowdown
	char mMessages[OBMQ_MESSAGE_QUEUE];
					// message buffer
	unsigned mCurrentMessage;	// message that currently is being displayed
	unsigned mNextMessage;		// message that will be displayed next
	char mCurrentMsgRepeat;		// counter for repeat of current message
	char mCurrentBit;		// 0..7 are for a message; 8 is inter-message
	char mCurrentInBit;		// in Msg: 0..BIT_LEN-1 are in bits;
					//         BIT_LEN..BIT_LEN+1 is inter-bit
					// InterMessage: 0..INTERMESSAGE_CLOCKS*2
	char mCurrentValue;		// the value that was last sent
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
//	infinite_repeat			boolean: repeat last element until a new one is queued?
void obmq_init(OneBitMessageQueue * m, void(*set_channel_value)(void*, char), void * set_channel_data, unsigned slowdown, unsigned repeat_message, char inter_message_clocks, char bitlength, char infinite_repeat);

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

