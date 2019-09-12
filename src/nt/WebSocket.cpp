#include "WebSocket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <event.h>
#include "log.h"
#include "sha1.h"
#include "base64.h"
#include "HttpRespHelper.h"

namespace nt
{

struct client
{
	int fd;
	struct bufferevent* buf_ev;
	WebSocket *obj;
};

static void free_client(struct client *client)
{
	bufferevent_free(client->buf_ev);
	free(client);
}


WebSocket::WebSocket()
{
	m_base = event_base_new();
	
	m_stop = false;

	m_loop_sleep_time = 1000;
	
	memset(m_pipe_fd, 0, sizeof(m_pipe_fd));
}
WebSocket::~WebSocket()
{
	Stop();
	
	if(m_pipe_fd[0] > 0)
	{
		close(m_pipe_fd[0]);
	}
	
	if(m_pipe_fd[1] > 0)
	{
		close(m_pipe_fd[1]);
	}
	
	event_base_free(m_base);
}

void WebSocket::SetLoopSleepTime(int t)
{
	m_loop_sleep_time = t;
}

int WebSocket::Start()
{
	if(pipe(m_pipe_fd) < 0)
	{
		err_log("pipe failed, errno:%d", errno);
		return -1;
	}
	
#ifdef WITH_BOOST
	thread th(boost::bind(&WebSocket::Run, this));
#else
	thread th(std::bind(&WebSocket::Run, this));
#endif // WITH_BOOST
	//event_reinit(m_base);
	th.detach();
	
	return 0;
}

void WebSocket::Stop()
{
	m_stop = true;
}

void WebSocket::Run()
{
	struct client* client = (struct client*)calloc(1, sizeof(*client));
	if (client == NULL)
	{
		err_log("malloc failed");
		return;
	}
	
	client->obj = this;
	client->fd = m_pipe_fd[0];

	client->buf_ev = bufferevent_new(client->fd,
		buf_pipe_read_callback,
		NULL,
		buf_pipe_error_callback,
		client);
	
	bufferevent_base_set(m_base, client->buf_ev);
	bufferevent_enable(client->buf_ev, EV_READ | EV_PERSIST);
	
	while (!m_stop)
	{
		event_base_dispatch(m_base);
		
		usleep(m_loop_sleep_time);
	}
	
}

int WebSocket::Notify(struct client* client, OP_TYPE type, void *data)
{
	char w_buf[1];
	w_buf[0] = 'c';
	
	OP_DATA op;
	op.type = type;
	op.data = data;
	op.client = client;
	m_op_queue.push(op);
	
	int size = write(m_pipe_fd[1], w_buf, 1);
	if(size <= 0)
	{
		return -1;
	}
	return 0;
}

int WebSocket::AddConnection(nt::SharedHttpInput_t input, int socket)
{
	const std::string *wsKey = input->GetHeader("Sec-WebSocket-Key");
	if(wsKey == NULL)
	{
		err_log("no 'Sec-WebSocket-Key' found, client ip:%s, uri:%s", input->GetRemoteAddr().c_str(), input->GetRequestURI().c_str());
		return -1;
	}
	
	std::string acceptKey;
	
	SHA1 sha;
	unsigned int message_digest[5];
	sha.Reset();
	sha << wsKey->c_str();
	sha << WebSocket_MAGIC_KEY;

	sha.Result(message_digest);
	for (int i = 0; i < 5; i++) 
	{
		message_digest[i] = htonl(message_digest[i]);
	}
	acceptKey = base64encode(reinterpret_cast<const char*>(message_digest),20);
	
	nt::SharedHttpOutput_t respOutput = nt::CreateSharedHttpOutput();
	respOutput->SetStatus(HTTP_SWITCHING_PROTOCOLS);
	respOutput->SetVersion("1.1");
	respOutput->SetHeader("Upgrade", "websocket");
	respOutput->SetHeader("Connection", "Upgrade");
	respOutput->SetHeader("Sec-WebSocket-Accept", acceptKey);
	
	nt::HttpRespHelper respHelper(respOutput);
	std::string pack;
	respHelper.CreateHeader(pack);

	//消息头
	send(socket, pack.c_str(), pack.size(), 0);
	
	info_log("websocket open, client ip:%s, uri:%s", input->GetRemoteAddr().c_str(), input->GetRequestURI().c_str());
	

	struct client* client = (struct client*)calloc(1, sizeof(*client));
	if (client == NULL)
	{
		err_log("malloc failed");
		return -2;
	}
	client->obj = this;
	client->fd = socket;
	client->buf_ev = bufferevent_new(client->fd,
		buf_read_callback,
		buf_write_callback,
		buf_error_callback,
		client);
		
	
	if(Notify(client, OP_ADD, NULL) != 0)
	{
		free_client(client);
		return -3;
	}
	
	Notify(client, OP_WRITE, new std::string("summer test web soket 20190819"));
	
	return 0;
}

void buf_read_callback(struct bufferevent* incoming, void* arg)
{
	const int BUF_SIZE = 4096;
	struct client* client = (struct client*)arg;
	//struct evbuffer* evreturn;

	char buf[BUF_SIZE] = { 0 };
	int n = bufferevent_read(client->buf_ev, buf, BUF_SIZE - 1);
	if (n == 0)
	{
		info_log("evbuffer_read end. close it\n");
		return;
	}
	else if (n < 0)
	{
		err_log("evbuffer_read fd:%d, errno:%d, strerror:%s", client->fd, errno, strerror(errno));
		return;
	}

	char buf2[1024] = { 0 };
	int nOutLen = 0;
	WsDecodeFrame(buf, n, buf2, nOutLen);

	debug_log("recv fd:%d, length:%d, response:%s\n", client->fd, nOutLen, buf2);

	//bufferevent_write(client->buf_ev, buf, n);

}

void buf_write_callback(struct bufferevent* bev, void* arg)
{
	struct client* client = (struct client*)arg;
	debug_log("buf_write_callback fd:%d\n", client->fd);
}

void buf_error_callback(struct bufferevent* bev, short what, void* arg)
{
	struct client* client = (struct client*)arg;
	info_log("fd:%d close, what:%d\n", client->fd, (int)what);

    close(client->fd);
    free_client(client);
}

void buf_pipe_read_callback(struct bufferevent* incoming, void* arg)
{
	const int BUF_SIZE = 1024;
	struct client* client = (struct client*)arg;
	//struct evbuffer* evreturn;

	char buf[BUF_SIZE] = { 0 };
	
	int n = 0;
	while( (n = bufferevent_read(client->buf_ev, buf, 1) ) > 0)
	{
		debug_log("recv fd:%d, length:%d, buf:%s\n", client->fd, n, buf);
		
		if(buf[0] != 'c')
		{
			err_log("unknow pipe cmd:%c", buf[0]);
			continue;
		}
		
		WebSocket *obj = client->obj;
		
		if(obj->m_op_queue.empty())
		{
			err_log("Notify but op_queue.empty");
			return;
		}
		
		OP_DATA op_data = obj->m_op_queue.front();
		obj->m_op_queue.pop();
		
		debug_log("OP_DATA type:%d", (int)op_data.type);
		
		struct client* to_client = (struct client*)op_data.client;
		
		switch(op_data.type)
		{
			case OP_ADD:
				bufferevent_base_set(obj->m_base, to_client->buf_ev);
				bufferevent_enable(to_client->buf_ev, EV_READ | EV_PERSIST);
				
				break;
			case OP_WRITE:
				{
					char buf2[WS_MAX_BUFFER+1] = {0};
					int nOutLen = 0;

					std::string *resp = (std::string *)op_data.data;
					
					WsEncodeFrame(resp->c_str(), resp->size(), buf2, nOutLen);
					
					int ret = bufferevent_write(to_client->buf_ev, buf2, nOutLen);
					if (ret != 0)
					{
						err_log("bufferevent_write failed:%d", ret);
					}

					debug_log("send fd:%d, data:%s", to_client->fd, resp->c_str());
					
					delete resp;
				}
				break;
			default:
				break;
		}
	}
}

void buf_pipe_error_callback(struct bufferevent* bev, short what, void* arg)
{
	struct client* client = (struct client*)arg;
	info_log("fd:%d close, what:%d\n", client->fd, (int)what);

    free_client(client);
}


int WsDecodeFrame(const char *msg, int nLen,char *bufOut,int& nOutLen)
{
	if(nLen < 2)
		return 0;
	int pos = 0;
	//unsigned char fin_ = (unsigned char)msg[pos] >> 7;
	//unsigned char  opcode_ = msg[pos] & 0x0f;
	pos++;
	unsigned char mask_ = (unsigned char)msg[pos] >> 7;
	unsigned char masking_key_[4];

	unsigned int payload_length_ = msg[pos] & 0x7f;
	pos++;
	if(payload_length_ == 126)
	{
		if(nLen<pos+2)
		{
			debug_log("wsReadData >>> nLen=%d<5",pos+2);
			return 0;
		}
		unsigned short length = 0;
		memcpy(&length, msg + pos, 2);
		pos += 2;
		payload_length_ = ntohs(length);
	}
	else if(payload_length_ == 127)
	{
		if(nLen<pos+4)
		{
			debug_log("wsReadData >>> nLen=%d<7",pos+4);
			return 0;
		}
		unsigned int length = 0;
		memcpy(&length, msg + pos, 4);
		pos += 4;
		payload_length_ = ntohl(length);
	}

	if(mask_ == 1)
	{
		if(nLen<pos+4)
			return 0;
		for(int i = 0; i < 4; i++)
			masking_key_[i] = msg[pos + i];
		pos += 4;
		
	}
	
	if ( payload_length_ > (WS_MAX_BUFFER-8) )
	{
		// 数据过长  
		return -1;  
	}
	
	int nRet = pos + payload_length_;
	if(nLen < nRet)
	{
		debug_log("wsReadData nLen=%d<nRet=%d",nLen,nRet);
		return 0;
	}
	if(mask_ != 1){
		memcpy(bufOut, msg + pos, payload_length_);
	}
	else 
	{
		for(int i = 0; i < (int)payload_length_; i++){
			int j = i % 4;
			bufOut[i] = msg[pos + i] ^ masking_key_[j];
		}
	}
	nOutLen = payload_length_;
	return nRet;
}

int WsEncodeFrame(const char* buf,int nLen, char* bufOut,int& nOutLen)  
{
	enum WS_FrameType frameType=WS_TEXT_FRAME;

	const unsigned int messageLength =nLen;  
 
	if ( messageLength > (WS_MAX_BUFFER-8) )
	{
		// 数据过长
		err_log("ERR::%s, data to long, messageLength:%u, maxbuff:%d", __func__, messageLength, (int)WS_MAX_BUFFER);
		return -1;  
	}  

	unsigned char payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;  
	// header: 2字节, mask位设置为0(不加密), 则后面的masking key无须填写, 省略4字节  
	unsigned char frameHeaderSize = 2 + payloadFieldExtraBytes;  
	unsigned char *frameHeader = new unsigned char[frameHeaderSize];  
	memset(frameHeader, 0, frameHeaderSize);  
	// fin位为1, 扩展位为0, 操作位为frameType  
	frameHeader[0] = static_cast<unsigned char>(0x80 | frameType);  

	// 填充数据长度  
	if (messageLength <= 0x7d)  
	{
		frameHeader[1] = static_cast<unsigned char>(messageLength);  
	}  
	else  
	{
		frameHeader[1] = 0x7e;  
		unsigned short len = htons(messageLength);  
		memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);  
	}  

	// 填充数据  
	unsigned int frameSize = frameHeaderSize + messageLength;  

	memcpy(bufOut, frameHeader, frameHeaderSize);  
	memcpy(bufOut + frameHeaderSize,buf, messageLength);  
	nOutLen=frameSize;
	delete[] frameHeader;  
	return nOutLen;  
} 



}// namespace nt
