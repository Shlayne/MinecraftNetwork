namespace net
{
	template<typename ID>
	Connection<ID>::Connection(Owner owner, asio::io_context& rContext, asio::ip::tcp::socket&& rrSocket, TSDeque<OwnedMessage<ID>>& rIncomingMessages)
		: m_Owner(owner), m_rContext(rContext), m_Socket(std::move(rrSocket)), m_rIncomingMessages(rIncomingMessages)
	{
		if (m_Owner == Owner::Server)
		{
			m_ValidationOut = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
			m_ValidationCheck = Encrypt(m_ValidationOut);
		}
	}

	template<typename ID>
	void Connection<ID>::ConnectToClient(IServer<ID>* pServer, uint32_t id)
	{
		if (m_Owner == Owner::Server)
		{
			if (IsConnected())
			{
				m_ID = id;
				WriteValidation();
				m_pServer = pServer;
				ReadValidation();
			}
		}
	}

	template<typename ID>
	void Connection<ID>::ConnectToServer(const asio::ip::tcp::resolver::results_type& crEndpoints)
	{
		if (m_Owner == Owner::Client)
		{
			asio::async_connect(m_Socket, crEndpoints,
			[this](asio::error_code error, asio::ip::tcp::endpoint endpoint)
			{
				if (!error)
					ReadValidation();
			});
		}
	}

	template<typename ID>
	void Connection<ID>::Disconnect()
	{
		if (IsConnected())
		{
			if (m_Owner == Owner::Server)
				m_pServer->Disconnect(this->shared_from_this());
			else
				asio::post(m_rContext, [this]() { m_Socket.close(); });
		}
	}

	template<typename ID>
	bool Connection<ID>::IsConnected() const
	{
		return m_Socket.is_open();
	}

	template<typename ID>
	uint32_t Connection<ID>::GetID() const
	{
		return m_ID;
	}

	template<typename ID>
	void Connection<ID>::Send(const Message<ID>& crMessage)
	{
		// TODO: use std::shared_ptr<Message<ID>> instead of const Message<ID>&
		// because this lambda COPIES THE ENTIRE MESSAGE, because it needs the
		// message to be alive whenever asio calls it.
		asio::post(m_rContext,
		[this, crMessage]()
		{
			bool alreadyWritingMessage = !m_OutgoingMessages.Empty();
			m_OutgoingMessages.PushBack(crMessage);
			if (!alreadyWritingMessage)
				WriteHeader();
		});
	}

	template<typename ID>
	uint64_t Connection<ID>::Encrypt(uint64_t data) const
	{
		// TODO: this is EXTREMELY EASY TO BRUTE-FORCE.
		// Although it works fine for testing, use a cryptography library or knowledge from CS classes.
		// Use public/private key thing.
		data ^= 0xDEADBEEFC0DECAFE;
		data = ((data & 0xF0F0F0F0F0F0F0F0) >> 4) | ((data & 0x0F0F0F0F0F0F0F0F) << 4);
		return data ^ 0xC0DEFACE12345678;
	}

	template<typename ID>
	void Connection<ID>::ReadHeader()
	{
		asio::async_read(m_Socket, asio::buffer(&m_TempIncomingMessage.header, sizeof(m_TempIncomingMessage.header)),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
			{
				if (m_TempIncomingMessage.header.size > 0)
				{
					m_TempIncomingMessage.body.resize(m_TempIncomingMessage.header.size);
					ReadBody();
				}
				else
					AddToIncomingMessageQueue();
			}
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Read header failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::ReadBody()
	{
		asio::async_read(m_Socket, asio::buffer(m_TempIncomingMessage.body.data(), m_TempIncomingMessage.body.size()),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
				AddToIncomingMessageQueue();
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Read body failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::ReadValidation()
	{
		asio::async_read(m_Socket, asio::buffer(&m_ValidationIn, sizeof(m_ValidationIn)),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
			{
				if (m_Owner == Owner::Server)
				{
					if (m_ValidationIn == m_ValidationCheck)
					{
						std::cout << "[SERVER] Client " << m_ID << " validated.\n";
						m_pServer->OnValidate(this->shared_from_this());
						ReadHeader();
					}
					else
					{
						std::cout << "[SERVER] Client " << m_ID << " invalidated.\n";
						m_pServer->OnInvalidate(this->shared_from_this());
						// Has to be async work so any messages the server implementation sends in
						// OnClientInvalidate can be sent before the connection is closed.
						Disconnect();
					}
				}
				else
				{
					m_ValidationOut = Encrypt(m_ValidationIn);
					WriteValidation();
				}
			}
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Read validation failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::WriteHeader()
	{
		asio::async_write(m_Socket, asio::buffer(&m_OutgoingMessages.Front().header, sizeof(m_OutgoingMessages.Front().header)),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
			{
				if (m_OutgoingMessages.Front().body.size() > 0)
					WriteBody();
				else
				{
					static_cast<void>(m_OutgoingMessages.PopFront());
					if (!m_OutgoingMessages.Empty())
						WriteHeader();
				}
			}
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Write header failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::WriteBody()
	{
		asio::async_write(m_Socket, asio::buffer(m_OutgoingMessages.Front().body.data(), m_OutgoingMessages.Front().body.size()),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
			{
				static_cast<void>(m_OutgoingMessages.PopFront());
				if (!m_OutgoingMessages.Empty())
					WriteHeader();
			}
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Write body failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::WriteValidation()
	{
		asio::async_write(m_Socket, asio::buffer(&m_ValidationOut, sizeof(m_ValidationOut)),
		[this](asio::error_code error, size_t size)
		{
			if (!error)
			{
				if (m_Owner == Owner::Client)
					ReadHeader();
			}
			else
			{
				std::cerr << '[' << (m_Owner == Owner::Server ? "SERVER" : "CLIENT") << "] Write validation failed for connection id " << m_ID << ": " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::AddToIncomingMessageQueue()
	{
		if (m_Owner == Owner::Server)
			static_cast<void>(m_rIncomingMessages.EmplaceBack(this->shared_from_this(), m_TempIncomingMessage));
		else
			static_cast<void>(m_rIncomingMessages.EmplaceBack(nullptr, m_TempIncomingMessage));

		m_TempIncomingMessage.body.clear();
		m_TempIncomingMessage.header.size = 0;

		ReadHeader();
	}
}
