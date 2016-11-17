// AsyncServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "AsyncServer.h"

//Ŭ���̾�Ʈ ID ������ �迭
int client_id_store[10];

CConnection::~CConnection()
{
}

void CConnection::set_callback(callback cb)
{
	m_cb = cb;
}

void CConnection::do_callback(const char* msg)
{
	m_cb(msg, m_nID);
}

std::string get_current_time()
{
	SYSTEMTIME curTime;
	GetLocalTime(&curTime);

	char temp_time[128];
	memset(temp_time, 0x00, sizeof(temp_time));

	sprintf_s(temp_time, "[%4d/%2d/%2d %02d:%02d:%02d]",
		curTime.wYear, curTime.wMonth, curTime.wDay,
		curTime.wHour, curTime.wMinute, curTime.wSecond);

	return temp_time;
}

std::string CConnection::user_join(int i)
{
	char temp[512];
	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%s (%d)���� �����ϼ̽��ϴ�.", get_current_time().c_str(), i);
	return temp;
}

std::string CConnection::user_exit(int i)
{
	char temp[512];
	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%s (%d)���� �����ϼ̽��ϴ�.", get_current_time().c_str(), i);
	return temp;
}

std::string CConnection::user_send_message(const char* msg, int i)
{
	char temp[512];
	memset(temp, 0x00, sizeof(temp));
	sprintf(temp, "%s(%d) %s", get_current_time().c_str(), i, msg);
	return temp;
}

void CConnection::start(int nID)
{
	m_nID = nID;

	cout << "Connected From : " << m_Socket.remote_endpoint().address() << endl;

	do_callback(user_join(m_nID).c_str());

	boost::asio::async_write(m_Socket, boost::asio::buffer(m_sMessage),
		boost::bind(&CConnection::handle_Accept, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));

	memset(m_szRecvBuf, 0x00, sizeof(m_szRecvBuf));
	m_Socket.async_receive(boost::asio::buffer(m_szRecvBuf, sizeof(m_szRecvBuf)),
		boost::bind(&CConnection::handle_Recv, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

void CConnection::handle_Wait_Recv(const boost::system::error_code& error, size_t recv)
{
	//if (!error)  // 0 �� ���� �������� ���� �÷��� 
	//{
	//	m_Socket.async_read_some(boost::asio::buffer(m_szRecvBuf),
	//		boost::bind(&CConnection::handle_Recv, shared_from_this(),
	//			boost::asio::placeholders::error,
	//			boost::asio::placeholders::bytes_transferred));
	//	cout << "handle_Wait_Recv : "<< m_szRecvBuf << endl;

	//}
	//else {
	//	std::cout << m_nID << " Disconnect(Read) : " << error.message() << std::endl;
	//}
}

void CConnection::handle_Accept(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/)
{

	//Define

}

void CConnection::handle_Recv(const boost::system::error_code& error, size_t t)
{
	if (!error)  // 0 �� ���� �������� ���� �÷��� 
	{
		// �޽����� ��������� 
		if (strlen(m_szRecvBuf) > 0)
		{
			//signal�� ������.
			do_callback(user_send_message(m_szRecvBuf, m_nID).c_str());
		}

		cout << m_nID << "] (" << m_szRecvBuf << ")" << endl;
		
		memset(m_szRecvBuf, 0x00, sizeof(m_szRecvBuf));

		//�޽��� ��� �� �Լ� ��ȣ�� �ݺ�
		m_Socket.async_read_some(boost::asio::buffer(m_szRecvBuf, sizeof(m_szRecvBuf)),
			boost::bind(&CConnection::handle_Recv, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

	}
	else {
		std::cout << m_nID << " Disconnect(Write) : " << error.message() << std::endl;
		del_signal(sig);

		//Ŭ���̾�Ʈ ���� ����
		m_Socket.shutdown_receive;
		do_callback(user_exit(m_nID).c_str());
		m_Socket.shutdown_send;
		m_Socket.close();
	}
}

void CConnection::add_signal(boost::signals2::signal<void(const char*, int)> & signal)
{
	connection = signal.connect(boost::bind(&CConnection::send, shared_from_this(), _1, _2));
}

void CConnection::del_signal(boost::signals2::signal<void(const char*, int)>& signal)
{
	signal.disconnect(connection);
}

tcp::socket& CConnection::socket()
{
	return m_Socket;
}

void CConnection::send(const char * msg, int id)
{

	if (id == m_nID)
		return;

	cout << "Send " << id << " -> " << m_nID << endl;

	int msg_len = (int)strlen(msg);
	if (msg_len > 0)
	{
		strcpy(m_szRecvBuf, msg);

		if (m_Socket.is_open()) {
			m_Socket.write_some(boost::asio::buffer(m_szRecvBuf, msg_len));
			memset(m_szRecvBuf, 0x00, sizeof(m_szRecvBuf));
		}
	}
}


//TCP ���� Ŭ����
/*

Function List
CTCP_Server(boost::asio::io_service& io, int port)
- �Ҹ���. WaitAccept()�Լ��� ȣ���Ͽ� client accept ����Ѵ�.

void WaitAccept()
- ���ؼ� ��ü�� �����ϰ� async_accept�Լ��� ����Ͽ� client accept ���. handle_Accept() ȣ��

void handle_Accept(CConnection::pointer new_connection, const boost::system::error_code& error)
- CConnection Ŭ������ start �Լ��� ȣ���ϰ� �ٽ� WaitAccept() �Լ��� ȣ���Ͽ� accept ����Ѵ�.

*/
class CTCP_Server
{
public:
	CTCP_Server(boost::asio::io_service& io, int port)
		: m_Acceptor(io, tcp::endpoint(tcp::v4(), port)), m_nAcceptCnt(0)
	{
		WaitAccept();
	}

private:
	tcp::acceptor m_Acceptor;
	int m_nAcceptCnt;

	void WaitAccept()
	{
		CConnection::pointer new_connection =
			CConnection::create(m_Acceptor.get_io_service());

		new_connection->set_callback(boost::bind(&CTCP_Server::send, this, _1, _2));

		m_Acceptor.async_accept(new_connection->socket(),
			boost::bind(&CTCP_Server::handle_Accept, this, new_connection,
				boost::asio::placeholders::error));

		m_nAcceptCnt++;
	}

	void handle_Accept(CConnection::pointer new_connection, const boost::system::error_code& error)
	{
		if (!error)
		{
			new_connection->start(m_nAcceptCnt);

			//���� �� Ŭ���̾�Ʈ�� signal ���Կ� ����Ѵ�.
			new_connection->add_signal(sig);

			WaitAccept();
		}
	}

	void send(const char* msg, int id)
	{
		sig(msg, id);
	}
};

int main(int argc, char** argv)
{
	try
	{
		//io ���� ��ü ����
		boost::asio::io_service io;

		//���� ����
		//CTCP_Server server(io, atoi(argv[1]));
		CTCP_Server server(io, 9600);

		//io �� ����
		io.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
