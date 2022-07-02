#pragma once

#include "Network/Common.h"

namespace net
{
	// The template ID of all of these structs and classes are for your
	// own enum class, but use IDType as the base, so MessageHeader is always 64 bits.
	using IDType = uint32_t;

#pragma pack(push, 1)
	template<typename ID>
	struct MessageHeader
	{
#if _HAS_CXX23
		static_assert(std::is_scoped_enum_v<ID> && std::is_same_v<std::underlying_type_t<ID>, IDType>,
#else
		static_assert(std::is_enum_v<ID> && !std::is_convertible_v<ID, IDType> && std::is_same_v<std::underlying_type_t<ID>, IDType>,
#endif
			"ID must be an enum class/struct in which the underlying type is net::IDType.");

		constexpr MessageHeader() noexcept = default;
		constexpr MessageHeader(ID id) noexcept;
		constexpr MessageHeader(const MessageHeader&) noexcept = default;
		constexpr MessageHeader(MessageHeader&&) noexcept = default;
		constexpr MessageHeader& operator=(const MessageHeader&) noexcept = default;
		constexpr MessageHeader& operator=(MessageHeader&&) noexcept = default;

		ID id{};
		uint32_t size = 0; // Excludes size of this header.
	};
#pragma pack(pop)

	template<typename ID>
	struct Message
	{
		Message() noexcept = default;
		Message(ID id) noexcept;
		Message(const Message& crMessage);
		Message(Message&& rrMessage) noexcept;
		Message& operator=(const Message& crMessage);
		Message& operator=(Message&& rrMessage) noexcept;

		MessageHeader<ID> header;
		std::vector<uint8_t> body;

		constexpr std::vector<uint8_t>::size_type Size() const;
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
