namespace net
{
	template<typename ID>
	DedicatedClient<ID>::DedicatedClient()
		: m_Socket(m_Context) {}

	template<typename ID>
	DedicatedClient<ID>::~DedicatedClient()
	{
		Disconnect();
	}

	template<typename ID>
	TSDeque<OwnedMessage<ID>>& DedicatedClient<ID>::GetIncomingMessageQueue()
	{
		return m_IncomingMessages;
	}

	template<typename ID>
	const TSDeque<OwnedMessage<ID>>& DedicatedClient<ID>::GetIncomingMessageQueue() const
	{
		return m_IncomingMessages;
	}

	template<typename ID>
	bool DedicatedClient<ID>::Connect(std::string_view address, uint16_t port)
	{
		if (IsConnected())
			Disconnect();

		try
		{
			asio::ip::tcp::resolver resolver(m_Context);
			auto endpoints = resolver.resolve(address, std::to_string(port));

			m_pConnection = std::make_unique<Connection<ID>>(
				ConnectionOwner::DedicatedClient, m_Context, asio::ip::tcp::socket(m_Context), m_IncomingMessages
			);

			m_pConnection->ConnectToServer(endpoints);
			m_ContextThread = std::thread([this]() { m_Context.run(); });

			return true;
		}
		catch (const std::exception& crException)
		{
			std::cerr << "DedicatedClient Exception: " << crException.what() << '\n';
			return false;
		}
	}

	template<typename ID>
	void DedicatedClient<ID>::Disconnect()
	{
		if (m_pConnection)
		{
			m_pConnection->Disconnect();
			m_Context.stop();
			if (m_ContextThread.joinable())
				m_ContextThread.join();
			m_pConnection.reset();
		}
	}

	template<typename ID>
	bool DedicatedClient<ID>::IsConnected() const
	{
		return m_pConnection && m_pConnection->IsConnected();
	}

	template<typename ID>
	void DedicatedClient<ID>::Send(const Message<ID>& crMessage)
	{
		if (IsConnected())
			m_pConnection->Send(crMessage);
	}
}
