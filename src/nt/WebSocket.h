#pragma once

#include <queue>
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "use_thread.h"
#include "HttpStatus.h"

struct event_base;
struct bufferevent;

namespace nt
{

#define WebSocket_MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_MAX_BUFFER (32768)

enum WS_FrameType
{
	WS_OPENING_FRAME,
	WS_TEXT_FRAME = 0x1,
	WS_BINARY_FRAME = 0x2,
	WS_EMPTY_FRAME = 0x3,
	WS_CLOSING_FRAME = 0x8,
	WS_PING_FRAME = 0x9,
	WS_PONG_FRAME = 0xA,
	WS_ERROR_FRAME,
};

int WsDecodeFrame(const char* msg, int nLen, char* bufOut, int& nOutLen);
int WsEncodeFrame(const char* buf, int nLen, char* bufOut, int& nOutLen);

void buf_error_callback(struct bufferevent* bev, short what, void* arg);
void buf_write_callback(struct bufferevent* bev, void* arg);
void buf_read_callback(struct bufferevent* incoming, void* arg);

void buf_pipe_read_callback(struct bufferevent* incoming, void* arg);
void buf_pipe_error_callback(struct bufferevent* bev, short what, void* arg);

enum OP_TYPE
{
	OP_ADD = 1,
	OP_READ = 2,
	OP_WRITE = 3,
};

struct OP_DATA
{
    OP_TYPE type;
	void *data;
	void *client;
};

class WebSocket
{
public:
	WebSocket();
	virtual ~WebSocket();
	
	int Start();

	int AddConnection(nt::SharedHttpInput_t input, int socket);

	void SetLoopSleepTime(int t);
	
	int Notify(struct client* client, OP_TYPE type, void *data);
private:
	friend void buf_error_callback(struct bufferevent* bev, short what, void* arg);
	friend void buf_write_callback(struct bufferevent* bev, void* arg);
	friend void buf_read_callback(struct bufferevent* incoming, void* arg);
	friend void buf_pipe_read_callback(struct bufferevent* incoming, void* arg);
    friend void buf_pipe_error_callback(struct bufferevent* bev, short what, void* arg);

	void Run();
	void Stop();

	event_base *m_base;
	bool m_stop;

	//microsecond
	int m_loop_sleep_time;
	
	int m_pipe_fd[2];
	std::queue<OP_DATA> m_op_queue;
};

};

