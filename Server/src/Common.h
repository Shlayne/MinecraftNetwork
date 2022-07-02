#pragma once

#include <Network.h>

enum class MessageType : net::IDType
{
	Server_ValidatedClient,
	Client_ValidatedServer,

	Client_PingServer,

	Client_MessageOtherClients,
};
