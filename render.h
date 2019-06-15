#pragma once

extern long ImGui_ImplWin32_WndProcHandler( HWND, u32, u32, long );
extern long __stdcall wndproc( HWND, u32, u32, long );

class c_render
{
public:
	enum callback_ctx_t : u8
	{
		CONTEXT_DRAW,
		CONTEXT_RESET,
		CONTEXT_RELEASE
	};

	struct callback_t
	{
		int m_ctx;
		const std::function<void( )> m_func;
	};

	bool setup( )
	{
		m_wndclass =
		{
			sizeof( WNDCLASSEX ),
			CS_CLASSDC,
			wndproc,
			0,
			0,
			GetModuleHandleA( 0 ),
			0,
			0,
			0,
			0,
			"random imgur",
			0
		};

		RegisterClassExA( &m_wndclass );

		m_window = CreateWindow( m_wndclass.lpszClassName, "random imgur", WS_OVERLAPPEDWINDOW, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) - 20, 0, 0, m_wndclass.hInstance, 0 );

		ShowWindow( m_window, SW_SHOWDEFAULT );
		UpdateWindow( m_window );

		if ( ( m_d3d9 = Direct3DCreate9( D3D_SDK_VERSION ) ) == nullptr )
		{
			release( );
			return false;
		}

		m_params = {};
		m_params.Windowed = true;
		m_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_params.BackBufferFormat = D3DFMT_UNKNOWN;
		m_params.EnableAutoDepthStencil = true;
		m_params.AutoDepthStencilFormat = D3DFMT_D16;
		m_params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

		if ( m_d3d9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_params, &m_device ) < 0 )
		{
			release( );
			return false;
		}
		
		IMGUI_CHECKVERSION( );

		ImGui::CreateContext( );
		{
			ImGuiIO& io = ImGui::GetIO( );
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Verdana.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Arial.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\Tahoma.ttf", 16.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );
			io.Fonts->AddFontFromFileTTF( "c:\\Windows\\Fonts\\comic.ttf", 24.0f, 0, io.Fonts->GetGlyphRangesDefault( ) );

			ImGui::StyleColorsDark( );
		}

		ImGui_ImplWin32_Init( m_window );
		ImGui_ImplDX9_Init( m_device );

		MSG msg{};
		while ( msg.message != WM_QUIT )
		{
			if ( PeekMessageA( &msg, 0, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessageA( &msg );
				continue;
			}

			ImGui_ImplDX9_NewFrame( );
			ImGui_ImplWin32_NewFrame( );

			ImGui::NewFrame( );

			for ( auto& callback : m_callbacks )
			{
				if ( callback.m_ctx == CONTEXT_DRAW )
					( callback.m_func )( );
			}

			ImGui::EndFrame( );

			m_device->SetRenderState( D3DRS_ZENABLE, false );
			m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, false );
			m_device->SetRenderState( D3DRS_SCISSORTESTENABLE, false );
			m_device->Clear( 0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA( 30, 30, 30, 255 ), 1.0f, 0 );

			if ( m_device->BeginScene( ) >= 0 )
			{
				ImGui::Render( );
				ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );
				m_device->EndScene( );
			}

			if ( m_device->Present( 0, 0, 0, 0 ) == D3DERR_DEVICELOST && m_device->TestCooperativeLevel( ) == D3DERR_DEVICENOTRESET )
			{
				for ( auto& callback : m_callbacks )
				{
					if ( callback.m_ctx == CONTEXT_RESET )
						( callback.m_func )( );
				}

				reset( );
			}
		}

		release( );
		return true;
	}

	void on_wndproc( u32 msg, u32 wide_param, long param )
	{
		if ( msg == WM_SIZE && wide_param != SIZE_MINIMIZED )
			reset( param );		
	}

	IDirect3DTexture9* create_texture( const u8a& image )
	{
		m_textures.push_back( nullptr );
		D3DXCreateTextureFromFileInMemoryEx( m_device, image.data( ), image.size( ), GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ) - 20, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, 0, 0, &m_textures.back( ) );

		return m_textures.back( );
	}

	void add_callback( const std::function< void( ) >& func, u8 type )
	{
		m_callbacks.push_back( { type, func } );
	}

private:
	void reset( long param = 0 )
	{
		ImGui_ImplDX9_InvalidateDeviceObjects( );

		if ( m_device )
			m_device->Reset( &m_params );

		if ( param != 0 )
		{
			m_params.BackBufferWidth = LOWORD( param );
			m_params.BackBufferHeight = HIWORD( param );
		}

		ImGui_ImplDX9_CreateDeviceObjects( );
	}

	void release( )
	{
		for ( auto& callback : m_callbacks )
		{
			if ( callback.m_ctx == CONTEXT_RELEASE )
				( callback.m_func )( );
		}

		ImGui_ImplDX9_Shutdown( );
		ImGui_ImplWin32_Shutdown( );
		ImGui::DestroyContext( );

		reset( );
		m_device = nullptr;

		if ( m_d3d9 )
		{
			m_d3d9->Release( );
			m_d3d9 = nullptr;
		}

		for ( auto& texture : m_textures )
		{
			if ( texture )
				texture->Release( );
		}

		m_textures.clear( );

		DestroyWindow( m_window );
		UnregisterClassA( m_wndclass.lpszClassName, m_wndclass.hInstance );
	}

	IDirect3DDevice9*	   m_device;
	IDirect3D9*			   m_d3d9;
	D3DPRESENT_PARAMETERS  m_params;
	WNDCLASSEX			   m_wndclass;
	HWND				   m_window;

	std::vector< IDirect3DTexture9* >  m_textures;
	std::vector< callback_t >		   m_callbacks;
}; inline c_render g_render;

long __stdcall wndproc( HWND hwnd, u32 msg, u32 wide_param, long param )
{
	if ( ImGui_ImplWin32_WndProcHandler( hwnd, msg, wide_param, param ) )
		return true;

	g_render.on_wndproc( msg, wide_param, param );

	switch ( msg )
	{
		case WM_SYSCOMMAND:
		{
			if ( ( wide_param & 0xfff0 ) == SC_KEYMENU )
				return false;

			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
			return false;
		}
	}

	return DefWindowProcA( hwnd, msg, wide_param, param );
}