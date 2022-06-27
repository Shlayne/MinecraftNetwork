#pragma once

#include <Network.h>

enum class MessageType : net::IDType
{
	Server_AcceptClient,
	Server_ValidatedClient,
	Server_InvalidatedClient,

	Client_PingServer,
	Client_MessageOtherClients,
	Client_GracefulDisconnect,
};
