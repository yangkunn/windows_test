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

//const std::string _MY_IP("172.22.32.213");	//õ���Ӵ�
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
		// �Է� ���� host�� resolving�Ѵ�. 
		tcp::resolver resolver(m_Socket.get_io_service());
		tcp::resolver::query query(_MY_IP, "9600");
		tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
		tcp::resolver::iterator end;

		// resolving�� endpoint�� ������ �õ��Ѵ�. 
		boost::system::error_code error = boost::asio::error::host_not_found;
		while (error && endpoint_iterator != end)
		{
			m_Socket.close();
			m_Socket.connect(*endpoint_iterator++, error);
		}

		// ���� �������� Ȯ�� 
		if (error)
			throw boost::system::system_error(error);

		m_bConnect = true;

		//// �о�� �����͸� ������ array�� �����. 
		//boost::array<CHAR, eBuffSize> buf;
		//size_t len = m_Socket.read_some(boost::asio::buffer(buf), error);
		//if (error == boost::asio::error::eof)
		//{
		//	m_bConnect = false;
		//	return;
		//}
		//else if (error)
		//	throw boost::system::system_error(error);

		// ���� �����͸� cout�� ����Ѵ�. 
		//std::cout.write(buf.data(), len) << endl;
	}

	bool IsRun() { return m_bConnect; }
	bool IsSocketOpen()
	{
		if (!m_Socket.is_open() && m_bConnect)   // Ŀ��Ʈ �� ���� ������ �������ȴٸ� 
		{
			m_bConnect = false;                 // Ŀ��Ʈ�� ���� ���� 
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
	//			//���� Ÿ��?
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
				//���� Ÿ��?
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

				//����ü�� ���� seq
				int seq = 0;

				//�Է��� ���ڿ��� ���̰� �ִ� ������� ũ��.
				if (buf.length() > MAX_LENGTH) {

					//���ڿ��� ��ġ�� ��Ÿ�� ������
					int position = 0;

					//���� �����ǰ� �Է��� ���ڿ��� �� ���̰� ���� ������
					while (position != buf.length()) {

						//����ü �ʱ�ȭ
						memset(&msg, 0x00, sizeof(Message));
						sprintf_s(msg.message, buf.substr(position, MAX_LENGTH-1).c_str());
						msg.message[MAX_LENGTH] = '\0';
						//!�� ������, #�� ������ �޽���
						if (buf.substr(position, MAX_LENGTH).length() < MAX_LENGTH-1)
							msg.is_last_message = '#';
						else
							msg.is_last_message = '!';

						msg.seq = ++seq;

						sprintf_s(msg.header, "%d", 
							strlen(msg.message) + sizeof(msg.seq) + sizeof(msg.is_last_message));

						//����ü ����
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

						//������ ���� ��ġ�� MAX_LENGTH��ŭ ����
						position += buf.substr(position, MAX_LENGTH-1).length();
					}
				}
				else
				{
					//����ü �ʱ�ȭ
					sprintf_s(msg.message, buf.c_str());
					msg.seq = 0;		//���� �������� �޽����� ������ 0
					msg.is_last_message = '#';	//���� �޽����� ������ ������ �޽�����('#')
					sprintf_s(msg.header, "%d", 
						strlen(msg.message) + sizeof(msg.seq) + sizeof(msg.is_last_message));

					cout << "Send Msg(" << seq << "):" << msg.message << endl;
					//����ü ����
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
		boost::asio::io_service io_service; // io_service�� �ϳ� �����Ѵ�. 

		CProtocol Ptc(io_service);
		boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));
		Ptc.Connect(); // ���� �õ� 
		boost::thread Recv(boost::bind(&CProtocol::handle_Recive, &Ptc));
		boost::thread Send(boost::bind(&CProtocol::handle_Send, &Ptc));
		io_service.run();

		while (Ptc.IsRun())
		{
		}

		Recv.join();
		Send.join();

		t.join();   // �����尡 ����� ������ ���� �Լ��� ���Ḧ ���´� 
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
