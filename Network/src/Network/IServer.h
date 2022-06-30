#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/Connection.h"

namespace net
{
	template<typename ID>
	class IServer
	{
	public:
		virtual ~IServer() = default;
	public:
		virtual bool Start() = 0;
		virtual void Stop() = 0;
		virtual void Update(size_t messageCount = -1, bool waitForMessage = false) = 0;
	public:
		virtual void MessageOne(std::shared_ptr<Connection<ID>> pConnection, const Message<ID>& crMessage) = 0;
		virtual void MessageAll(const Message<ID>& crMessage, std::shared_ptr<Connection<ID>> pIgnoredConnection = nullptr) = 0;
		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection) = 0;
	public:
		virtual void OnValidate(std::shared_ptr<Connection<ID>> pConnection) = 0;
		virtual void OnInvalidate(std::shared_ptr<Connection<ID>> pConnection) = 0;
	protected:
		// Return false to reject the connection, true to approve.
		virtual bool OnConnect(std::shared_ptr<Connection<ID>> pConnection) = 0;
		virtual void OnDisconnect(std::shared_ptr<Connection<ID>> pConnection) = 0;
		virtual void OnMessage(std::shared_ptr<Connection<ID>> pConnection, Message<ID>& rMessage) = 0;
	};
}
