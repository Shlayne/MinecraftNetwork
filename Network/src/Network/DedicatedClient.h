#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/Connection.h"
#include "Network/IClient.h"

namespace net
{
	template<typename ID>
	class DedicatedClient : public IClient<ID>
	{
	public:
		DedicatedClient();
		virtual ~DedicatedClient();
	public:
		virtual bool Connect(std::string_view address, uint16_t port) override final;
		virtual void Disconnect() override final;
		virtual bool IsConnected() const override final;
	public:
		virtual void PollMessages(size_t messageCount = -1, bool waitForMessage = false) override final;
		virtual void Send(const Message<ID>& crMessage) override final;
	private:
		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection) override final;
	private:
		asio::io_context m_Context;
		std::thread m_ContextThread;
		asio::ip::tcp::socket m_Socket;
		std::shared_ptr<Connection<ID>> m_pConnection;
		TSDeque<OwnedMessage<ID>> m_IncomingMessages;
	private:
		friend std::ostream& operator<< <ID>(std::ostream& rOstream, const DedicatedClient<ID>& crDedicatedClient);
	};

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const DedicatedClient<ID>& crDedicatedClient);
}

#include "Network/DedicatedClient.inl"
