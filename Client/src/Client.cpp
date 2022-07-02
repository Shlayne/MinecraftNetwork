#include "Common.h"
#include <Windows.h>

class DedicatedClient : public net::DedicatedClient<MessageType>
{
public:
	void PingServer()
	{
		net::Message<MessageType> message = MessageType::Client_PingServer;
		message << std::chrono::system_clock::now();
		std::cout << *this << " Pinging server...\n";
		Send(message);
	}

	void MessageOtherClients()
	{
		net::Message<MessageType> message = MessageType::Client_MessageOtherClients;
		std::cout << *this << " Messaging other clients...\n";
		Send(message);
	}
protected:
	virtual void OnValidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << *this << " Validated server.\n";
	}

	virtual void OnInvalidate(std::shared_ptr<net::Connection<MessageType>> pConnection) override
	{
		std::cout << *this << " Invalidated server.\n";
	}
};

int main()
{
	std::cout << "Enter IP Address: ";
	std::string address;
	std::getline(std::cin, address);

	std::cout << "Enter Port Number: ";
	std::string portString;
	std::getline(std::cin, portString);
	uint16_t port = 0;
	try
	{
		int result = std::stoi(portString);
		port = static_cast<uint16_t>(result);

		if (result != port)
			std::cerr << "Warning actual port number used: " << +port << ".\n";
	}
	catch (const std::exception& crException)
	{
		std::cerr << "Failed to convert port number to uint16_t: " << crException.what() << "\nPress any key to continue...";
		std::cin.get();
		return -1;
	}

	DedicatedClient client;
	client.Connect(address, port);

	bool pKeys[3]{ false, false, false };
	bool pOldKeys[3]{ false, false, false };

	while (client.IsConnected())
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			pKeys[0] = !!(GetAsyncKeyState('1') & 0x8000);
			pKeys[1] = !!(GetAsyncKeyState('2') & 0x8000);
			pKeys[2] = !!(GetAsyncKeyState('3') & 0x8000);
		}

		if (pKeys[0] && !pOldKeys[0]) client.PingServer();
		if (pKeys[1] && !pOldKeys[1]) client.MessageOtherClients();
		if (pKeys[2] && !pOldKeys[2]) client.Disconnect();

		for (uint32_t i = 0; i < 3; i++)
			pOldKeys[i] = pKeys[i];

		if (!client.GetIncomingMessageQueue().Empty())
		{
			net::Message<MessageType> message = client.GetIncomingMessageQueue().PopFront().message;
			switch (message.header.id)
			{
				case MessageType::Client_PingServer:
				{
					std::chrono::system_clock::time_point start;
					message >> start;
					std::cout << client << " Ping: " << std::chrono::duration<double>(std::chrono::system_clock::now() - start).count() << '\n';
					break;
				}
				case MessageType::Client_MessageOtherClients:
				{
					uint32_t id;
					message >> id;
					std::cout << client << " Message received from client " << id << ".\n";
					break;
				}
				case MessageType::Server_ValidatedClient:
				{
					std::cout << client << " Server validated connection.\n";
					break;
				}
			}
		}
	}

	client.Disconnect();
	std::cout << client << " Disconnected.\n";
	std::cin.get();

	return 0;
}
