#pragma once

#include "Network/Common.h"
#include "Network/Message.h"
#include "Network/TSDeque.h"
#include "Network/IServer.h"

namespace net
{
	template<typename ID>
	class DedicatedServer;

	//template<typename ID>
	//class PeerToPeerNode;

	template<typename ID>
	class Connection : public std::enable_shared_from_this<Connection<ID>>
	{
	public:
		enum class Owner { Server, Client }; // This will DEFINETLY be changed/removed.
	public:
		Connection(Owner owner, asio::io_context& rContext, asio::ip::tcp::socket&& rrSocket, TSDeque<OwnedMessage<ID>>& rIncomingMessages);
	public:
		void ConnectToClient(IServer<ID>* pServer, uint32_t id);
		void ConnectToServer(const asio::ip::tcp::resolver::results_type& crEndpoints);
		void Disconnect();
		bool IsConnected() const;
	public:
		uint32_t GetID() const;
		void Send(const Message<ID>& crMessage);
	public:
		uint64_t Encrypt(uint64_t data) const;
	private:
		void ReadHeader();
		void ReadBody();
		void ReadValidation();
		void WriteHeader();
		void WriteBody();
		void WriteValidation();

		void AddToIncomingMessageQueue();
	private:
		Owner m_Owner;
		asio::io_context& m_rContext;
		asio::ip::tcp::socket m_Socket;
		TSDeque<Message<ID>> m_OutgoingMessages;
		TSDeque<OwnedMessage<ID>>& m_rIncomingMessages;
		Message<ID> m_TempIncomingMessage;
		uint32_t m_ID = 0;

		// DedicatedClient -> nullptr
		// DecicatedServer -> DecicatedServer*
		// PeerToPeerNode  -> PeerToPeerNode*
		IServer<ID>* m_pServer = nullptr;
	private:
		uint64_t m_ValidationOut = 0;
		uint64_t m_ValidationIn = 0;
		uint64_t m_ValidationCheck = 0;
	};
}

#include "Network/Connection.inl"
