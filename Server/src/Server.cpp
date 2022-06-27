#include "Common.h"

class DedicatedServer : public net::DedicatedServer<MessageType>
{
public:
	DedicatedServer(uint16_t port = 0) : net::DedicatedServer<MessageType>(port) {}
public:
	virtual void OnValidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		net::Message<MessageType> message = MessageType::Server_ValidatedClient;
		message << pConnection->GetID();
		pConnection->Send(message);
	}

	virtual void OnInvalidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		net::Message<MessageType> message = MessageType::Server_InvalidatedClient;
		pConnection->Send(message);
	}
protected:
	virtual bool OnConnect(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		net::Message<MessageType> message = MessageType::Server_AcceptClient;
		pConnection->Send(message);

		// Could implement exclude list by checking the client's ip address.
		return true;
	}

	virtual void OnDisconnect(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << "[SERVER] Client " << pConnection->GetID() << " disconnected";
		if (pConnection->DisconnectedGracefully())
			std::cout << " gracefully";
		std::cout << ".\n";
	}

	virtual void OnMessage(std::shared_ptr<net::Connection<MessageType>> pConnection, net::Message<MessageType>& rMessage) override
	{
		switch (rMessage.header.id)
		{
			case MessageType::Client_PingServer:
			{
				std::cout << "[SERVER] Client " << pConnection->GetID() << " ping bouncing.\n";
				pConnection->Send(rMessage);
				break;
			}
			case MessageType::Client_MessageOtherClients:
			{
				std::cout << "[SERVER] Client " << pConnection->GetID() << " messaging other clients.\n";
				rMessage << pConnection->GetID();
				MessageAll(rMessage, pConnection);
				break;
			}
			case MessageType::Client_GracefulDisconnect:
			{
				pConnection->Disconnect(true);
				break;
			}
		}
	}
};

int main()
{
	DedicatedServer server;
	server.Start();

	while (true)
		server.Update(-1, true);

	std::cout << "[SERVER] Closed.\n";
	std::cin.get();

	return 0;
}
