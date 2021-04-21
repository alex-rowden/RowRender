#pragma once

#include <Windows.h>
#include <VersionHelpers.h>
#include <wlanapi.h>
#pragma comment(lib, "Wlanapi.lib")
#include "rpc/server.h"
#include "rpc/client.h"
#include "rpc/this_handler.h"
#include <rpc/rpc_error.h>
#include "RowRender.h"


int NetworkMapper(bool isServer, int floor = 1);
