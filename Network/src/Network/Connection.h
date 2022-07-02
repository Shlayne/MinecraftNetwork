#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/Connection.h"
#include "Network/TSDeque.h"
#include "Network/IConnectable.h"

namespace net
{
	enum class ConnectionOwner { DedicatedServer, DedicatedClient, PeerToPeerNode };

	template<typename ID>
	class Connection : public std::enable_shared_from_this<Connection<ID>>
	{
	public:
		Connection(ConnectionOwner owner, asio::io_context& rContext, asio::ip::tcp::socket&& rrSocket, TSDeque<OwnedMessage<ID>>& rIncomingMessages);
	public:
		void ConnectToClient(IConnectable<ID>* pConnectable, uint64_t id = 0);
		void ConnectToServer(IConnectable<ID>* pConnectable, const asio::ip::tcp::resolver::results_type& crEndpoints);
		void Disconnect();
		bool IsConnected() const;
	public:
		ConnectionOwner GetOwner() const;
		uint64_t GetID() const;
		void Send(const Message<ID>& crMessage);
	public:
		uint64_t Encrypt(uint64_t data) const;
	private:
		void ReadHeader();
		void ReadBody();
		void ReadValidation1();
		void ReadValidation2();
		void ReadValidation3();
		void ReadValidation4();
		void WriteHeader();
		void WriteBody();
		void WriteValidation1();
		void WriteValidation2();
		void WriteValidation3();
		void WriteValidation4();

		void AddToIncomingMessageQueue();
	private:
		ConnectionOwner m_Owner;
		asio::io_context& m_rContext;
		asio::ip::tcp::socket m_Socket;
		TSDeque<Message<ID>> m_OutgoingMessages;
		TSDeque<OwnedMessage<ID>>& m_rIncomingMessages;
		Message<ID> m_TempIncomingMessage;
		uint64_t m_ID = 0;
		IConnectable<ID>* m_pConnectable = nullptr;
	private:
		uint64_t m_ValidationIn = 0;
		uint64_t m_ValidationOut = 0;
		uint64_t m_ValidationCheck = 0;
	};

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const Connection<ID>& crConnection);
}

#include "Network/Connection.inl"
