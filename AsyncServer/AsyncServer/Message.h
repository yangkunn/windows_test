#pragma once

#include <string>
using namespace std;

typedef struct __MESSAGE__
{
	short seq;
	char is_last_message;
	char header[4 + 1];
	char message[1350 + 1];
} Message;

//typedef boost::shared_ptr<Message> message_ptr;
//deque<message_ptr> message_queue;

deque<Message> message_queue;

//class Message
//{
//public:
//	enum { header_length = 4 };	//헤더 길이
//	enum { max_body_length = 1024 + sizeof(short) }; //메시지 길이
//
//	Message()
//		: body_length_(0)
//	{
//	}
//
//	const char* data() const
//	{
//		return data_;
//	}
//
//	char* data()
//	{
//		return data_;
//	}
//
//	std::size_t length() const
//	{
//		return header_length + body_length_;
//	}
//
//	const char* body() const
//	{
//		return data_ + header_length;
//	}
//
//	char* body()
//	{
//		return data_ + header_length;
//	}
//
//	std::size_t body_length() const
//	{
//		return body_length_;
//	}
//
//	void body_length(std::size_t new_length)
//	{
//		body_length_ = new_length;
//		if (body_length_ > max_body_length)
//			body_length_ = max_body_length;
//	}
//
//	bool decode_header()
//	{
//		char header[header_length + 1] = "\0";	// ""
//		strncat_s(header, data_, header_length);
//		body_length_ = std::atoi(header);
//		if (body_length_ > max_body_length)
//		{
//			body_length_ = 0;
//			return false;
//		}
//		return true;
//	}
//
//	void encode_header()
//	{
//		char header[header_length + 1] = "\0";
//		sprintf_s(header, "%4d", static_cast<int>(body_length_));
//		std::memcpy(data_, header, header_length);
//	}
//
//	void reset_data()
//	{
//		memset(data_, 0x00, sizeof(data_));
//	}
//
//private:
//	short seq;
//	char data_[header_length + max_body_length];
//	std::size_t body_length_;
//};