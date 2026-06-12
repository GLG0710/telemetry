#pragma 

struct AckState {
	char senseLine[256];
	char last[64];
	unsigned long timestamp = 0;
};
extern AckState ack;