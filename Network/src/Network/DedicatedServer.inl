namespace net
{
	template<typename ID>
	DedicatedServer<ID>::DedicatedServer(uint16_t port)
		: m_Acceptor(m_Context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		, m_Port(m_Acceptor.local_endpoint().port())
	{

	}

	template<typename ID>
	DedicatedServer<ID>::~DedicatedServer()
	{
		Stop();
	}

	template<typename ID>
	bool DedicatedServer<ID>::Start()
	{
		try
		{
			WaitForClientConnection();
			m_ContextThread = std::thread([this]() { m_Context.run(); });

			std::cout << "[SERVER] Started on port " << +m_Port << ".\n";
			return true;
		}
		catch (const std::exception& crException)
		{
			std::cerr << "[SERVER] Exception while starting: " << crException.what() << '\n';
			return false;
		}
	}

	template<typename ID>
	void DedicatedServer<ID>::Stop()
	{
		m_Context.stop();
		if (m_ContextThread.joinable())
			m_ContextThread.join();

		std::cout << "[SERVER] Stopped.\n";
	}

	template<typename ID>
	void DedicatedServer<ID>::Update(size_t messageCount, bool waitForMessage)
	{
		if (waitForMessage)
			m_IncomingMessages.Wait();

		while (messageCount-- && !m_IncomingMessages.Empty())
		{
			OwnedMessage<ID> message = std::move(m_IncomingMessages.PopFront());
			OnMessage(message.remoteConnection, message.message);
		}
	}

	template<typename ID>
	uint16_t DedicatedServer<ID>::GetPort() const
	{
		return m_Port;
	}

	template<typename ID>
	void DedicatedServer<ID>::MessageOne(std::shared_ptr<Connection<ID>> pConnection, const Message<ID>& crMessage)
	{
		if (pConnection && pConnection->IsConnected())
			pConnection->Send(crMessage);
		else
			Disconnect(pConnection);
	}

	template<typename ID>
	void DedicatedServer<ID>::MessageAll(const Message<ID>& crMessage, std::shared_ptr<Connection<ID>> pClientIgnored)
	{
		bool clientsRemoved = false;

		for (auto& rpClient : m_Clients)
		{
			if (rpClient != nullptr && rpClient->IsConnected())
			{
				if (rpClient != pClientIgnored)
					rpClient->Send(crMessage);
			}
			else
			{
				clientsRemoved = true;
				OnDisconnect(rpClient);
				rpClient.reset();
			}
		}

		if (clientsRemoved)
			std::erase_if(m_Clients, [](const auto& crpClient) { return !(crpClient != nullptr && crpClient->IsConnected()); });
	}

	template<typename ID>
	void DedicatedServer<ID>::Disconnect(std::shared_ptr<Connection<ID>> pConnection, bool graceful)
	{
		// TODO: the whole graceful disconnect is a race condition, figure it out or remove it.
		pConnection->m_DisconnectedGracefully = graceful;
		OnDisconnect(pConnection);
		pConnection.reset();
		std::erase_if(m_Clients, [](const auto& crpClient) { return !(crpClient != nullptr && crpClient->IsConnected()); });
	}

	template<typename ID>
	void DedicatedServer<ID>::OnValidate(std::shared_ptr<Connection<ID>> pConnection) {}

	template<typename ID>
	void DedicatedServer<ID>::OnInvalidate(std::shared_ptr<Connection<ID>> pConnection) {}

	template<typename ID>
	bool DedicatedServer<ID>::OnConnect(std::shared_ptr<Connection<ID>> pConnection) { return true; }

	template<typename ID>
	void DedicatedServer<ID>::OnDisconnect(std::shared_ptr<Connection<ID>> pConnection) {}

	template<typename ID>
	void DedicatedServer<ID>::OnMessage(std::shared_ptr<Connection<ID>> pConnection, Message<ID>& rMessage) {}

	template<typename ID>
	void DedicatedServer<ID>::WaitForClientConnection()
	{
		m_Acceptor.async_accept([this](asio::error_code error, asio::ip::tcp::socket socket)
		{
			if (!error)
			{
				std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << '\n';

				std::shared_ptr<Connection<ID>> newConnection = std::make_shared<Connection<ID>>(
					Connection<ID>::Owner::Server, m_Context, std::move(socket), m_IncomingMessages
				);

				if (OnConnect(newConnection))
				{
					m_Clients.push_back(std::move(newConnection));
					m_Clients.back()->ConnectToClient(this, m_NextClientID);
					std::cout << "[SERVER] Client " << m_NextClientID++ << " connected.\n";
				}
				else
					std::cout << "[SERVER] Connection denied.\n";
			}
			else
				std::cerr << "[SERVER] New connection error: " << error.message() << '\n';

			WaitForClientConnection();
		});
	}
}
