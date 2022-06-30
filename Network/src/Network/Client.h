#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/Connection.h"

namespace net
{
	template<typename ID>
	class Client
	{
	public:
		Client();
		virtual ~Client();
	public:
		inline TSDeque<OwnedMessage<ID>>& GetIncomingMessageQueue();
		inline const TSDeque<OwnedMessage<ID>>& GetIncomingMessageQueue() const;
	public:
		bool Connect(std::string_view address, uint16_t port);
		void Disconnect();
		bool IsConnected() const;
	public:
		void Send(const Message<ID>& crMessage);
	private:
		asio::io_context m_Context;
		std::thread m_ContextThread;
		asio::ip::tcp::socket m_Socket;
		std::unique_ptr<Connection<ID>> m_pConnection;
		TSDeque<OwnedMessage<ID>> m_IncomingMessages;
	};
}

#include "Network/Client.inl"
