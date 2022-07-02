#pragma once

#include <Network.h>

enum class MessageType : net::IDType
{
	Client_PingServer,
	Client_MessageOtherClients,
};
