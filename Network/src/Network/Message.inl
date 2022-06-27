namespace net
{
	template<typename ID>
	MessageHeader<ID>::MessageHeader(ID id)
		: id(id) {}

	template<typename ID>
	Message<ID>::Message(MessageHeader<ID> header)
		: header(header) {}

	template<typename ID>
	Message<ID>::Message(ID id)
		: header(id) {}

	template<typename ID>
	Message<ID>::Message(const Message& crMessage)
		: header(crMessage.header), body(crMessage.body) {}

	template<typename ID>
	Message<ID>::Message(Message&& rrMessage) noexcept
		: header(std::move(rrMessage.header)), body(std::move(rrMessage.body)) {}

	template<typename ID>
	Message<ID>& Message<ID>::operator=(const Message& crMessage)
	{
		if (this != &crMessage)
		{
			header = crMessage.header;
			body = crMessage.body;
		}
		return *this;
	}

	template<typename ID>
	Message<ID>& Message<ID>::operator=(Message&& rrMessage) noexcept
	{
		if (this != &rrMessage)
		{
			header = std::move(rrMessage.header);
			body = std::move(rrMessage.body);
		}
		return *this;
	}

	template<typename ID>
	constexpr size_t Message<ID>::Size() const
	{
		return header.size;
	}

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const Message<ID>& crMessage)
	{
		return rOstream << "ID=" << +static_cast<IDType>(crMessage.header.id) << " Size=" << +crMessage.Size();
	}

	template<typename ID, typename Data>
	Message<ID>& operator<<(Message<ID>& rMessage, const Data& crData)
	{
		static_assert(std::is_trivially_copyable_v<Data>, "Data is not trivially copyable.");
		assert(rMessage.header.size + sizeof(Data) >= rMessage.header.size && "Attempted to write too much data to a message.");

		size_t oldBodySize = rMessage.body.size();
		rMessage.body.resize(oldBodySize + sizeof(crData));
		memcpy_s(rMessage.body.data() + oldBodySize, sizeof(Data), &crData, sizeof(Data));
		rMessage.header.size += sizeof(Data);
		return rMessage;
	}

	template<typename ID, typename Data>
	Message<ID>& operator>>(Message<ID>& rMessage, Data& rData)
	{
		static_assert(std::is_trivially_copyable_v<Data>, "Data is not trivially copyable.");
		assert(rMessage.body.size() >= sizeof(Data) && "Attempted to read too much data from a message.");

		size_t newBodySize = rMessage.body.size() - sizeof(Data);
		memcpy_s(&rData, sizeof(Data), rMessage.body.data() + newBodySize, sizeof(Data));
		rMessage.body.resize(newBodySize);
		rMessage.header.size -= sizeof(Data);
		return rMessage;
	}

	template<typename ID>
	OwnedMessage<ID>::OwnedMessage(const std::shared_ptr<Connection<ID>>& crRemoteConnection, const Message<ID>& crMessage)
		: remoteConnection(crRemoteConnection), message(crMessage) {}

	template<typename ID>
	OwnedMessage<ID>::OwnedMessage(std::shared_ptr<Connection<ID>>&& rrRemoteConnection, Message<ID>&& rrMessage)
		: remoteConnection(std::move(rrRemoteConnection)), message(std::move(rrMessage)) {}

	template<typename ID>
	OwnedMessage<ID>::OwnedMessage(const OwnedMessage& crOwnedMessage)
		: remoteConnection(crOwnedMessage.remoteConnection), message(crOwnedMessage.message) {}

	template<typename ID>
	OwnedMessage<ID>::OwnedMessage(OwnedMessage&& rrOwnedMessage) noexcept
		: remoteConnection(std::move(rrOwnedMessage.remoteConnection)), message(std::move(rrOwnedMessage.message)) {}

	template<typename ID>
	OwnedMessage<ID>& OwnedMessage<ID>::operator=(const OwnedMessage& crOwnedMessage)
	{
		if (this != &crOwnedMessage)
		{
			remoteConnection = crOwnedMessage.remoteConnection;
			message = crOwnedMessage.message;
		}
		return *this;
	}

	template<typename ID>
	OwnedMessage<ID>& OwnedMessage<ID>::operator=(OwnedMessage&& rrOwnedMessage) noexcept
	{
		if (this != &rrOwnedMessage)
		{
			remoteConnection = std::move(rrOwnedMessage.remoteConnection);
			message = std::move(rrOwnedMessage.message);
		}
		return *this;
	}

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const OwnedMessage<ID>& crOwnedMessage)
	{
		return rOstream << crOwnedMessage.message;
	}
}
