#include "Common.h"

class DedicatedServer : public net::DedicatedServer<MessageType>
{
public:
	DedicatedServer(uint16_t port = 0) : net::DedicatedServer<MessageType>(port) {}
public:
	virtual void OnValidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << *pConnection << " Validated.\n";
	}

	virtual void OnInvalidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << *pConnection << " Invalidated.\n";
	}
protected:
	virtual bool OnConnect(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		// Could implement exclude list by checking the client's ip address.
		return true;
	}

	virtual void OnDisconnect(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << *pConnection << " Disconnected.\n";
	}

	virtual void OnMessage(std::shared_ptr<net::Connection<MessageType>> pConnection, net::Message<MessageType>& rMessage) override
	{
		switch (rMessage.header.id)
		{
			case MessageType::Client_PingServer:
			{
				std::cout << *pConnection << " Ping bouncing.\n";
				pConnection->Send(rMessage);
				break;
			}
			case MessageType::Client_MessageOtherClients:
			{
				std::cout << *pConnection << " Messaging other clients.\n";
				rMessage << pConnection->GetID();
				MessageAll(rMessage, pConnection);
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
