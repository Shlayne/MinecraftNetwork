#include "Connection.h"
namespace net
{
	template<typename ID>
	Connection<ID>::Connection(ConnectionOwner owner, asio::io_context& rContext, asio::ip::tcp::socket&& rrSocket, TSDeque<OwnedMessage<ID>>& rIncomingMessages)
		: m_Owner(owner), m_rContext(rContext), m_Socket(std::move(rrSocket)), m_rIncomingMessages(rIncomingMessages) {}

	template<typename ID>
	void Connection<ID>::ConnectToClient(IConnectable<ID>* pConnectable, uint64_t id)
	{
		if (IsConnected())
		{
			if (m_Owner == ConnectionOwner::DedicatedServer)
			{
				m_pConnectable = pConnectable;
				m_ID = id;

				m_ValidationOut = static_cast<uint64_t>(std::chrono::system_clock::now().time_since_epoch().count());
				m_ValidationCheck = Encrypt(m_ValidationOut);
				WriteValidation1();
			}
		}
	}

	template<typename ID>
	void Connection<ID>::ConnectToServer(IConnectable<ID>* pConnectable, const asio::ip::tcp::resolver::results_type& crEndpoints)
	{
		if (m_Owner == ConnectionOwner::DedicatedClient)
		{
			m_pConnectable = pConnectable;

			asio::async_connect(m_Socket, crEndpoints,
			[this](asio::error_code error, asio::ip::tcp::endpoint endpoint)
			{
				if (!error)
					ReadValidation1();
				else
				{
					std::cerr << *this << " Connection failed.\n";
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::Disconnect()
	{
		if (IsConnected() && m_pConnectable != nullptr)
			m_pConnectable->Disconnect(this->shared_from_this());
		else
			asio::post(m_rContext, [this]() { m_Socket.close(); });
	}

	template<typename ID>
	bool Connection<ID>::IsConnected() const
	{
		return m_Socket.is_open();
	}

	template<typename ID>
	ConnectionOwner Connection<ID>::GetOwner() const
	{
		return m_Owner;
	}

	template<typename ID>
	uint64_t Connection<ID>::GetID() const
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

		switch (m_Owner)
		{
			case ConnectionOwner::DedicatedServer:
			case ConnectionOwner::DedicatedClient:
				data ^= 0xDEADBEEFC0DECAFE;
				data = ((data & 0xF0F0F0F0F0F0F0F0) >> 4) | ((data & 0x0F0F0F0F0F0F0F0F) << 4);
				data ^= 0xC0DEFACE12345678;
				break;
			// Have peer to peer nodes use different encryption to make sure
			// they can't connect to servers and clients can't connect to them.
			case ConnectionOwner::PeerToPeerNode:
				data ^= 0xC0DECAFEDEADBEEF;
				data = ((data & 0xF0F0F0F0F0F0F0F0) >> 4) | ((data & 0x0F0F0F0F0F0F0F0F) << 4);
				data ^= 0x12345678C0DEFACE;
				break;
		}

		return data;
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
				std::cerr << *this << " Read header failed: " << error.message() << '\n';
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
				std::cerr << *this << " Read body failed: " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::ReadValidation1()
	{
		if (m_Owner == ConnectionOwner::DedicatedClient)
		{
			asio::async_read(m_Socket, asio::buffer(&m_ValidationIn, sizeof(m_ValidationIn)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
				{
					m_ValidationOut = Encrypt(m_ValidationIn);
					m_ValidationCheck = Encrypt(m_ValidationOut);
					WriteValidation2();
				}
				else
				{
					std::cerr << *this << " Read validation 1 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::ReadValidation2()
	{
		if (m_Owner == ConnectionOwner::DedicatedServer)
		{
			asio::async_read(m_Socket, asio::buffer(&m_ValidationIn, sizeof(m_ValidationIn)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
				{
					if (m_ValidationIn == m_ValidationCheck)
					{
						m_pConnectable->OnValidate(this->shared_from_this());

						// Successfully validated client, so validate self.
						m_ValidationOut = Encrypt(m_ValidationIn);
						WriteValidation3();
					}
					else
					{
						m_pConnectable->OnInvalidate(this->shared_from_this());
						// Has to be async work so any messages the server implementation sends in
						// OnClientInvalidate can be sent before the connection is closed.
						Disconnect();
					}
				}
				else
				{
					std::cerr << *this << " Read validation 2 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::ReadValidation3()
	{
		if (m_Owner == ConnectionOwner::DedicatedClient)
		{
			asio::async_read(m_Socket, asio::buffer(&m_ValidationIn, sizeof(m_ValidationIn)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
				{
					if (m_ValidationIn == m_ValidationCheck)
						ReadValidation4();
					else
					{
						m_pConnectable->OnInvalidate(this->shared_from_this());
						Disconnect();
					}
				}
				else
				{
					std::cerr << *this << " Read validation 3 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::ReadValidation4()
	{
		if (m_Owner == ConnectionOwner::DedicatedClient)
		{
			asio::async_read(m_Socket, asio::buffer(&m_ID, sizeof(m_ID)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
				{
					m_pConnectable->OnValidate(this->shared_from_this());
					ReadHeader();
				}
				else
				{
					std::cerr << *this << " Read validation 4 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
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
				std::cerr << *this << " Write header failed: " << error.message() << '\n';
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
				std::cerr << *this << " Write body failed: " << error.message() << '\n';
				Disconnect();
			}
		});
	}

	template<typename ID>
	void Connection<ID>::WriteValidation1()
	{
		if (m_Owner == ConnectionOwner::DedicatedServer)
		{
			asio::async_write(m_Socket, asio::buffer(&m_ValidationOut, sizeof(m_ValidationOut)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
					ReadValidation2();
				else
				{
					std::cerr << *this << " Write validation 1 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::WriteValidation2()
	{
		if (m_Owner == ConnectionOwner::DedicatedClient)
		{
			asio::async_write(m_Socket, asio::buffer(&m_ValidationOut, sizeof(m_ValidationOut)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
					ReadValidation3();
				else
				{
					std::cerr << *this << " Write validation 2 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::WriteValidation3()
	{
		if (m_Owner == ConnectionOwner::DedicatedServer)
		{
			asio::async_write(m_Socket, asio::buffer(&m_ValidationOut, sizeof(m_ValidationOut)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
					WriteValidation4();
				else
				{
					std::cerr << *this << " Write validation 3 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::WriteValidation4()
	{
		if (m_Owner == ConnectionOwner::DedicatedServer)
		{
			asio::async_write(m_Socket, asio::buffer(&m_ID, sizeof(m_ID)),
			[this](asio::error_code error, size_t size)
			{
				if (!error)
				{
					// Assume successful bidirectional validation, so begin normal operation.
					// If client invalidates us, asio will have this fail and disconnect will be called.
					ReadHeader();
				}
				else
				{
					std::cerr << *this << " Write validation 4 failed: " << error.message() << '\n';
					Disconnect();
				}
			});
		}
	}

	template<typename ID>
	void Connection<ID>::AddToIncomingMessageQueue()
	{
		if (m_Owner != ConnectionOwner::DedicatedClient)
			static_cast<void>(m_rIncomingMessages.EmplaceBack(this->shared_from_this(), m_TempIncomingMessage));
		else
			static_cast<void>(m_rIncomingMessages.EmplaceBack(nullptr, m_TempIncomingMessage));

		m_TempIncomingMessage.body.clear();
		m_TempIncomingMessage.header.size = 0;

		ReadHeader();
	}

	template<typename ID>
	std::ostream& operator<<(std::ostream& rOstream, const Connection<ID>& crConnection)
	{
		switch (crConnection.GetOwner())
		{
			case ConnectionOwner::DedicatedServer: rOstream << "[SERVER>CLIENT" << crConnection.GetID() << ']'; break;
			case ConnectionOwner::DedicatedClient: rOstream << "[CLIENT" << crConnection.GetID() << ">SERVER]"; break;
			// TODO: Figure out ID system for P2P network.
			case ConnectionOwner::PeerToPeerNode: rOstream << "[P2PNODE" << crConnection.GetID() << ">P2PNODE]"; break;
		}

		return rOstream;
	}
}
