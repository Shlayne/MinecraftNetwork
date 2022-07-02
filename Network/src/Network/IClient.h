#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/IConnectable.h"

namespace net
{
	template<typename ID>
	class IClient : public IConnectable<ID>
	{
	public:
		virtual ~IClient() = default;
	public:
		virtual TSDeque<OwnedMessage<ID>>& GetIncomingMessageQueue() = 0;
		virtual const TSDeque<OwnedMessage<ID>>& GetIncomingMessageQueue() const = 0;
	public:
		virtual bool Connect(std::string_view address, uint16_t port) = 0;
		virtual void Disconnect() = 0;
		virtual bool IsConnected() const = 0;
	public:
		virtual void Send(const Message<ID>& crMessage) = 0;
	};
}
