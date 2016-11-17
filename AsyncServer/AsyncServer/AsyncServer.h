#pragma warning(disable : 4996)

#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <set>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Message.h"

using boost::asio::ip::tcp;
using namespace std;

#define MAX_BUFFER_LENGTH 1024
#define max_server 5

typedef boost::shared_ptr<Message> message_ptr;
deque<message_ptr> message_queue;

typedef struct __CLIENT_INFO__
{
	int  idx;
	char nickname[128];
	char time[64];
	int  length;
	char* message;
}CLIENT_INFO;

boost::signals2::signal<void(const char*, int)> sig;
typedef boost::function<void(const char*, int)> callback;

class CConnection : public boost::enable_shared_from_this<CConnection>
{
public:
	CConnection(boost::asio::io_service& io) : m_Socket(io), m_nID(-1)
	{
	}
	virtual ~CConnection();

	typedef boost::shared_ptr<CConnection> pointer;

	void start(int nID);

	static pointer create(boost::asio::io_service& io)
	{
		return pointer(new CConnection(io));
	}

	void add_signal(boost::signals2::signal<void(const char*, int)>& signal);	//accept 할때 추가하는 역할
	void del_signal(boost::signals2::signal<void(const char*, int)>& signal);	//client가 접속 종료될때 
	
	tcp::socket& socket();

	void send(const char* msg, int id);

	void set_callback(callback cb);
	void do_callback(const char*);

	std::string user_join(int i);		//유저 입장시 
	std::string user_exit(int i);		//유저 퇴장시
	std::string user_send_message(const char* msg, int i);	//유저에게 메시지 보낼 시


private:
	tcp::socket m_Socket;
	int m_nID;

	CLIENT_INFO m_userInfo;
	boost::signals2::connection connection;

	callback m_cb;

	//boost::array<BYTE, eBuffSize> m_RecvBuf;
	char m_szRecvBuf[MAX_BUFFER_LENGTH];
	char m_szSendBuf[MAX_BUFFER_LENGTH];
	std::string m_sMessage;

	void handle_Accept(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/);

	void handle_Wait_Recv(const boost::system::error_code& error, size_t /*bytes_transferred*/);
	void handle_Recv(const boost::system::error_code& error, size_t /*bytes_transferred*/);
};
