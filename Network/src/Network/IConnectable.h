#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/Connection.h"

namespace net
{
	template<typename ID>
	class IConnectable
	{
	public:
		virtual void PollMessages(size_t messageCount = -1, bool waitForMessage = false) = 0;
	protected:
		friend class Connection<ID>;

		virtual ~IConnectable() = default;

		virtual bool AllowConnection(std::shared_ptr<Connection<ID>> pConnection) { return true; }

		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection) = 0;

		virtual void OnConnect(std::shared_ptr<Connection<ID>> pConnection) {}
		virtual void OnDisconnect(std::shared_ptr<Connection<ID>> pConnection) {}

		virtual void OnValidate(std::shared_ptr<Connection<ID>> pConnection) {}
		virtual void OnInvalidate(std::shared_ptr<Connection<ID>> pConnection) {}

		virtual void OnMessage(std::shared_ptr<Connection<ID>> pConnection, Message<ID>& rMessage) {}
	};
}
