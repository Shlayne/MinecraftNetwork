#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/Connection.h"
#include "Network/IConnectable.h"

namespace net
{
	template<typename ID>
	class IServer : public IConnectable<ID>
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
	};
}
