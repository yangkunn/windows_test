// ASyncClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <ctime>
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "Message.h"

#define MAX_LENGTH 1024

//const std::string _MY_IP("172.22.32.213");	//천선임님
const std::string _MY_IP("172.22.32.212");

using namespace std;
using boost::asio::ip::tcp;

class CProtocol
{
private:

	enum EEnum
	{
		eBuffSize = 128,
	};

	tcp::socket m_Socket;
	bool m_bConnect;

	int m_nTestCount;

	boost::mutex mutex_;

public:

	CProtocol(boost::asio::io_service& io) : m_Socket(io)
	{
		m_bConnect = false;
		m_nTestCount = 0;
	}

	~CProtocol() {}

	void Connect()
	{
		// 입력 받은 host을 resolving한다. 
		tcp::resolver resolver(m_Socket.get_io_service());
		tcp::resolver::query query(_MY_IP, "9600");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// resolving된 endpoint로 접속을 시도한다. 
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			m_Socket.close();
			m_Socket.connect(*endpoint_iterator++, error);
		}

		// 접속 실패인지 확인 
		if (error)
			throw boost::system::system_error(error);

		m_bConnect = true;

		//// 읽어올 데이터를 저장할 array를 만든다. 
		//boost::array<CHAR, eBuffSize> buf;
		//size_t len = m_Socket.read_some(boost::asio::buffer(buf), error);
		//if (error == boost::asio::error::eof)
		//{
		//	m_bConnect = false;
		//	return;
		//}
		//else if (error)
		//	throw boost::system::system_error(error);

		// 받은 데이터를 cout로 출력한다. 
		//std::cout.write(buf.data(), len) << endl;
	}

	bool IsRun() { return m_bConnect; }
	bool IsSocketOpen()
	{
		if (!m_Socket.is_open() && m_bConnect)   // 커넥트 된 이후 소켓이 닫혀버렸다면 
		{
			m_bConnect = false;                 // 커넥트도 끊김 판정 
			return false;
		}

		return true;
	}

	//void handle_Recive()
	//{
	//	Message msg;
	//	while (m_bConnect)
	//	{
	//		if (!IsSocketOpen())
	//			break;

	//		try
	//		{
	//			char buf[1024];
	//			memset(&buf, 0x00, sizeof(buf));

	//			int len = m_Socket.receive(boost::asio::buffer(buf, sizeof(buf)));


	//			{
	//				boost::mutex::scoped_lock(mutex);
	//				if (len <= 0)
	//				{
	//					cout << "Recv error." << endl;
	//				}
	//				else
	//					cout << buf << endl;
	//			}
	//		}
	//		catch (std::exception& e)
	//		{
	//			//여길 타나?
	//			m_bConnect = false;
	//			std::cerr << e.what() << std::endl;
	//		}
	//	}

	//}

	void handle_Recive()
	{
		Message msg;
		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			try
			{
				memset(&msg, 0x00, sizeof(Message));

				int len = m_Socket.receive(boost::asio::buffer((void*)&msg, sizeof(Message)));


				{
					boost::mutex::scoped_lock(mutex_);
					if (len <= 0)
					{
						cout << "Recv error." << endl;
					}
					else
						cout << msg.message << endl;
				}
			}
			catch (std::exception& e)
			{
				//여길 타나?
				m_bConnect = false;
				std::cerr << e.what() << std::endl;
			}
		}
	}

	void handle_Send()
	{
		string buf;

		Message msg;

		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			buf.clear();

			try
			{
				boost::system::error_code error;
				//{
				//	boost::mutex::scoped_lock(mutex_);

				//	getline(cin, buf, '\n');
				//}

				getchar();

				buf.clear();
				for (int i = 0; i < 10000; i++)
				{
					//buf += i;
					buf = buf + "-";
				}

				cout << buf << endl;

				memset(&msg, 0x00, sizeof(Message));

				if (!buf.compare("exit") || !buf.compare("EXIT")) {
					m_Socket.close();
					m_bConnect = false;
					break;
				}
				else if (buf.length() == 0)
					continue;

				//구조체에 보낼 seq
				int seq = 0;

				//입력한 문자열의 길이가 최대 사이즈보다 크면.
				if (buf.length() > MAX_LENGTH) {

					//문자열의 위치를 나타낼 포인터
					int position = 0;

					//현재 포지션과 입력한 문자열의 총 길이가 같을 때까지
					while (position != buf.length()) {

						//구조체 초기화
						memset(&msg, 0x00, sizeof(Message));
						sprintf_s(msg.message, buf.substr(position, MAX_LENGTH-1).c_str());
						msg.message[MAX_LENGTH] = '\0';
						//!는 진행중, #은 마지막 메시지
						if (buf.substr(position, MAX_LENGTH).length() < MAX_LENGTH-1)
							msg.is_last_message = '#';
						else
							msg.is_last_message = '!';

						msg.seq = ++seq;

						sprintf_s(msg.header, "%d", 
							strlen(msg.message) + sizeof(msg.seq) + sizeof(msg.is_last_message));

						//구조체 전송
						//int len = m_Socket.write_some(boost::asio::buffer(buf.c_str(), buf.length()), error);
						int len = m_Socket.write_some(boost::asio::buffer((void*)&msg, sizeof(Message)), error);

						cout << "Send Msg(" << seq << "):" << msg.message << endl;

						if (error == boost::asio::error::eof) {
							cout << "write Fail" << len << endl;
							m_Socket.close();
							m_bConnect = false;
							break;
						}
						else
							cout << "Write Success : " << len << endl;

						//버퍼의 현재 위치를 MAX_LENGTH만큼 조정
						position += buf.substr(position, MAX_LENGTH-1).length();
					}
				}
				else
				{
					//구조체 초기화
					sprintf_s(msg.message, buf.c_str());
					msg.seq = 0;		//단일 사이즈의 메시지는 무조건 0
					msg.is_last_message = '#';	//단일 메시지는 무조건 마지막 메시지임('#')
					sprintf_s(msg.header, "%d", 
						strlen(msg.message) + sizeof(msg.seq) + sizeof(msg.is_last_message));

					cout << "Send Msg(" << seq << "):" << msg.message << endl;
					//구조체 전송
					//int len = m_Socket.write_some(boost::asio::buffer(buf.c_str(), buf.length()), error);
					int len = m_Socket.write_some(boost::asio::buffer((char*)&msg, sizeof(Message)), error);

					if (error == boost::asio::error::eof) {
						cout << "write Fail" << len << endl;
						m_Socket.close();
						m_bConnect = false;
						break;
					}
					else
						cout << "Write Success : " << len << endl;

				}

				m_nTestCount++;
			}
			catch (std::exception& e)
			{
				m_bConnect = false;
				std::cerr << e.what() << std::endl;
			}
		}


	}
};

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service; // io_service를 하나 생성한다. 

		CProtocol Ptc(io_service);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
		Ptc.Connect(); // 접속 시도 
		boost::thread Recv(boost::bind(&CProtocol::handle_Recive, &Ptc));
		boost::thread Send(boost::bind(&CProtocol::handle_Send, &Ptc));
		io_service.run();

		while (Ptc.IsRun())
		{
		}

		Recv.join();
		Send.join();

		t.join();   // 쓰레드가 종료될 때까지 메인 함수의 종료를 막는다 
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	int in;
	std::cout << "END";
	std::cin >> in;

	return 0;
}
