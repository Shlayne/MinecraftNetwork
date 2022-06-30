#include "Common.h"
#include <Windows.h>

class Client : public net::Client<MessageType>
{
public:
	void PingServer()
	{
		net::Message<MessageType> message = MessageType::Client_PingServer;
		message << std::chrono::system_clock::now();
		std::cout << "[CLIENT:" << GetID() << "] Pinging server...\n";
		Send(message);
	}

	void MessageOtherClients()
	{
		net::Message<MessageType> message = MessageType::Client_MessageOtherClients;
		std::cout << "[CLIENT:" << GetID() << "] Messaging other clients...\n";
		Send(message);
	}

	// Local copy of assigned id.
public:
	uint32_t GetID() const { return m_ID; }
	void SetID(uint32_t id) { m_ID = id; }
private:
	uint32_t m_ID = 0;
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

	Client client;
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
					std::cout << "[CLIENT:" << client.GetID() << "] Ping: " << std::chrono::duration<double>(std::chrono::system_clock::now() - start).count() << '\n';
					break;
				}
				case MessageType::Client_MessageOtherClients:
				{
					uint32_t id;
					message >> id;
					std::cout << "[CLIENT:" << client.GetID() << "] Message received from client " << id << ".\n";
					break;
				}
				case MessageType::Server_AcceptClient:
				{
					std::cout << "[CLIENT] Server accepted connection.\n";
					break;
				}
				case MessageType::Server_ValidatedClient:
				{
					uint32_t id;
					message >> id;
					client.SetID(id);
					std::cout << "[CLIENT:" << client.GetID() << "] Server validated connection.\n";
					break;
				}
				case MessageType::Server_InvalidatedClient:
				{
					std::cout << "[CLIENT] Server invalidated connection.\n";
					break;
				}
			}
		}
	}

	client.Disconnect();
	std::cout << "[CLIENT] Disconnected.\n";
	std::cin.get();

	return 0;
}
