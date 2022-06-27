#pragma once

#include "Network/Common.h"

namespace net
{
	// The template ID of all of these structs and classes are for your
	// own enum class, but use IDType as the base, so MessageHeader is always 64 bits.
	using IDType = uint32_t;

	template<typename ID>
	struct MessageHeader
	{
#if _HAS_CXX23
		static_assert(std::is_scoped_enum_v<ID> && std::is_same_v<std::underlying_type_t<ID>, IDType>,
#else
		static_assert(std::is_enum_v<ID> && !std::is_convertible_v<ID, IDType> && std::is_same_v<std::underlying_type_t<ID>, IDType>,
#endif
			"ID must be an enum class/struct in which the underlying type is net::IDType.");

		MessageHeader() = default;
		MessageHeader(ID id);
		MessageHeader(const MessageHeader&) = default;
		MessageHeader(MessageHeader&&) noexcept = default;
		MessageHeader& operator=(const MessageHeader&) = default;
		MessageHeader& operator=(MessageHeader&&) noexcept = default;

		ID id{};
		uint32_t size = 0; // Excludes size of this header.
	};

	template<typename ID>
	struct Message
	{
		Message() = default;
		Message(MessageHeader<ID> header);
		Message(ID id);
		Message(const Message& crMessage);
		Message(Message&& rrMessage) noexcept;
		Message& operator=(const Message& crMessage);
		Message& operator=(Message&& rrMessage) noexcept;

		MessageHeader<ID> header;
		std::vector<uint8_t> body;

		constexpr size_t Size() const;
	};

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const Message<ID>& crMessage);

	template<typename ID, typename Data>
	Message<ID>& operator<<(Message<ID>& rMessage, const Data& crData);

	template<typename ID, typename Data>
	Message<ID>& operator>>(Message<ID>& rMessage, Data& rData);

	template<typename ID>
	class Connection;

	template<typename ID>
	struct OwnedMessage
	{
		OwnedMessage() = default;
		OwnedMessage(const std::shared_ptr<Connection<ID>>& crRemoteConnection, const Message<ID>& crMessage);
		OwnedMessage(std::shared_ptr<Connection<ID>>&& rrRemoteConnection, Message<ID>&& rrMessage);
		OwnedMessage(const OwnedMessage& crOwnedMessage);
		OwnedMessage(OwnedMessage&& rrOwnedMessage) noexcept;
		OwnedMessage& operator=(const OwnedMessage& crOwnedMessage);
		OwnedMessage& operator=(OwnedMessage&& rrOwnedMessage) noexcept;

		std::shared_ptr<Connection<ID>> remoteConnection = nullptr;
		Message<ID> message;
	};

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const OwnedMessage<ID>& crOwnedMessage);
}

#include "Network/Message.inl"
