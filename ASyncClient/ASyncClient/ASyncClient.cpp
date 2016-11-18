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

//const std::string _MY_IP("172.22.32.213");	//õ���Ӵ�
const std::string _MY_IP("172.22.32.212");

using namespace std;
using boost::asio::ip::tcp;

typedef struct __CLIENT_INFO__
{
	int  idx;				//4
	char nickname[128];		//128
	char time[64];			//64
	int  length;			//4
	char message;			//4
}CLIENT_INFO;

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

	void handle_Recive()
	{
		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			try
			{
				char buf[1024];
				memset(&buf, 0x00, sizeof(buf));

				int len = m_Socket.receive(boost::asio::buffer(buf, sizeof(buf)));


				{
					boost::mutex::scoped_lock(mutex);
					if (len <= 0)
					{
						cout << "Recv error." << endl;
					}
					else
						cout << buf << endl;
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

		while (m_bConnect)
		{
			if (!IsSocketOpen())
				break;

			buf.clear();

			try
			{
				boost::system::error_code error;
				{
					boost::mutex::scoped_lock(mutex);

					getline(cin, buf, '\n');
				}

				if (!buf.compare("exit") || !buf.compare("EXIT")) {
					m_Socket.close();
					m_bConnect = false;
					break;
				}
				else if (buf.length() == 0)
					continue;

				int len = m_Socket.write_some(boost::asio::buffer(buf.c_str(), buf.length()), error);

				if (error == boost::asio::error::eof) {
					cout << "write Fail" << len << endl;
					m_Socket.close();
					m_bConnect = false;
					break;
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
