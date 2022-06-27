#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/Connection.h"

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
		virtual void Update(size_t messageCount = -1, bool waitForMessage = false) override final;
		uint16_t GetPort() const;
	public:
		virtual void MessageOne(std::shared_ptr<Connection<ID>> pConnection, const Message<ID>& crMessage) override final;
		virtual void MessageAll(const Message<ID>& crMessage, std::shared_ptr<Connection<ID>> pConnectionIgnored = nullptr) override final;
		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection, bool graceful = false) override final;
	public:
		virtual void OnValidate(std::shared_ptr<Connection<ID>> pConnection) override;
		virtual void OnInvalidate(std::shared_ptr<Connection<ID>> pConnection) override;
	protected:
		// Return false to reject the connection, true to approve.
		virtual bool OnConnect(std::shared_ptr<Connection<ID>> pConnection) override;
		virtual void OnDisconnect(std::shared_ptr<Connection<ID>> pConnection) override;
		virtual void OnMessage(std::shared_ptr<Connection<ID>> pConnection, Message<ID>& rMessage) override;
	private:
		void WaitForClientConnection();
	private:
		asio::io_context m_Context;
		std::thread m_ContextThread;
		asio::ip::tcp::acceptor m_Acceptor;
		uint16_t m_Port = 0;
		TSDeque<OwnedMessage<ID>> m_IncomingMessages;
		std::deque<std::shared_ptr<Connection<ID>>> m_Clients;
		uint32_t m_NextClientID = 1;
	};
}

#include "Network/DedicatedServer.inl"
