#pragma once
#include <string>
#include <algorithm>
#include <memory>
#include <fstream>
#include <thread>
#include <array>
#include <functional>
#include <experimental/filesystem>

#include <d3d9.h>
#include <d3dx9.h>
#include <dwmapi.h>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#include "./imgui/imgui.h"
#include "./imgui/examples/imgui_impl_dx9.h"
#include "./imgui/examples/imgui_impl_win32.h"

#include "types.h"
#include "hash.h"
#include "util.h"

class c_context
{
public:
	std::vector< std::wstring > m_previous_urls;
}; inline c_context ctx;

#include "networking.h"
#include "render.h"
#include "ui.h"