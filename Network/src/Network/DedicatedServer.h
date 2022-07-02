#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/Connection.h"
#include "Network/IServer.h"

namespace net
{
	template<typename ID>
	class DedicatedServer : public IServer<ID>
	{
	public:
		DedicatedServer(uint16_t port = 0);
		virtual ~DedicatedServer();
	public:
		virtual bool Start() override final;
		virtual void Stop() override final;
		uint16_t GetPort() const;
	public:
		virtual void PollMessages(size_t messageCount = -1, bool waitForMessage = false) override final;
		virtual void MessageOne(std::shared_ptr<Connection<ID>> pConnection, const Message<ID>& crMessage) override final;
		virtual void MessageAll(const Message<ID>& crMessage, std::shared_ptr<Connection<ID>> pConnectionIgnored = nullptr) override final;
	private:
		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection) override final;
	private:
		void WaitForClientConnection();
	private:
		asio::io_context m_Context;
		std::thread m_ContextThread;
		asio::ip::tcp::acceptor m_Acceptor;
		uint16_t m_Port = 0;
		TSDeque<OwnedMessage<ID>> m_IncomingMessages;
		std::deque<std::shared_ptr<Connection<ID>>> m_Clients;
		uint64_t m_NextClientID = 1;
	};
}

#include "Network/DedicatedServer.inl"
