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
		virtual ~IConnectable() = default;
	protected:
		friend class Connection<ID>;
		virtual void Disconnect(std::shared_ptr<Connection<ID>> pConnection) = 0;
	protected:
		// Can't message from these two.
		virtual void OnValidate(std::shared_ptr<Connection<ID>> pConnection) {}
		virtual void OnInvalidate(std::shared_ptr<Connection<ID>> pConnection) {}
	protected:
		// Return false to reject the connection, true to approve.
		virtual bool OnConnect(std::shared_ptr<Connection<ID>> pConnection) { return true; }
		virtual void OnDisconnect(std::shared_ptr<Connection<ID>> pConnection) {}
		virtual void OnMessage(std::shared_ptr<Connection<ID>> pConnection, Message<ID>& rMessage) {}
	};
}
