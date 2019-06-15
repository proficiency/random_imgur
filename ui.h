#pragma once

namespace ui
{
	class c_child
	{
	public:
		c_child( IDirect3DTexture9* image, const std::string& title ) : m_image( image ), m_title( title ), m_open( true ) {}

		void draw_elements( int index )
		{
			ImGui::Begin( ( m_title + "##" + std::to_string( index ) ).c_str( ), &m_open, ImGuiWindowFlags_NoCollapse );
			ImGui::Image( m_image, ImVec2( ImGui::GetWindowSize( ).x - ( ImGui::GetStyle( ).WindowPadding.x * 2 ), ImGui::GetWindowSize( ).y * 0.95f ) );
			ImGui::End( );
		}

		bool open( )
		{
			return m_open;
		}

	private:
		IDirect3DTexture9*  m_image;
		bool				m_open;
		std::string			m_title;
	};

	inline void draw( )
	{
		static IDirect3DTexture9*		image = g_render.create_texture( g_networking.download_image( ) );
		static bool						show_styleeditor = false;
		static std::vector< c_child >	children = {};
		std::string					    url = util::unicode_to_ascii( ctx.m_previous_urls.back( ) );

		ImGui::Begin( "random imgur", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_MenuBar );

		// let user customize colors and stuff, should prolly parse a file so they can save themes
		{
			if ( ImGui::BeginMenuBar( ) )
			{
				if ( ImGui::BeginMenu( "theme" ) )
				{
					ImGui::ShowStyleSelector( " " );

					if ( ImGui::Button( "advanced" ) )
						show_styleeditor = true;

					ImGui::EndMenu( );
				}

				ImGui::EndMenuBar( );
			}

			if ( show_styleeditor )
			{
				ImGui::Begin( "style editor", &show_styleeditor );
				ImGui::ShowStyleEditor( );
				ImGui::End( );
			}
		}

		// let user copy or open the url
		if ( !ctx.m_previous_urls.empty( ) )
		{
			ImGui::InputText( "", ( char* ) url.c_str( ), url.length( ), ImGuiInputTextFlags_ReadOnly );

			ImGui::SameLine( );
			if ( ImGui::Button( "copy" ) )
				util::to_clipboard( url );

			ImGui::SameLine( );
			if ( ImGui::Button( "open in browser" ) )
				ShellExecuteA( 0, "open", util::unicode_to_ascii( ctx.m_previous_urls.back( ) ).c_str( ), 0, 0, SW_SHOWNORMAL );
		}

		// go to the next image
		if ( ImGui::Button( "next" ) )
			image = g_render.create_texture( g_networking.download_image( ) );

		ImGui::SameLine( );

		// go to the previous image
		if ( ctx.m_previous_urls.size( ) > 1 && ImGui::Button( "previous" ) )
		{
			// verry p
			image = g_render.create_texture( g_networking.download_image( ctx.m_previous_urls[ctx.m_previous_urls.size( ) - 2] ) );
			ctx.m_previous_urls.erase( ctx.m_previous_urls.begin( ) + ctx.m_previous_urls.size( ) - 1 );
		}

		ImGui::SameLine( );

		// save image to file
		if ( ImGui::Button( "save" ) )
			D3DXSaveTextureToFileA( std::string( url.substr( strlen_ct( "https://i.imgur.com/" ) ) ).c_str( ), D3DXIFF_JPG, image, 0 );

		ImGui::SameLine( );

		// open image in another window
		if ( ImGui::Button( "open in new window" ) )
			children.push_back( c_child( image, url ) );

		if ( ctx.m_previous_urls.size( ) > 1 )
		{
			ImGui::SameLine( );
			ImGui::Text( "%i images viewed", ctx.m_previous_urls.size( ) );
		}

		ImGui::Image( image, ImVec2( ImGui::GetWindowSize( ).x - ( ImGui::GetStyle( ).WindowPadding.x * 2 ), ImGui::GetWindowSize( ).y * 0.85f ) );
		ImGui::End( );

		// draw and maintain each child
		for ( u32 i = 0; i < children.size( ); ++i )
		{
			if ( !children[i].open( ) )
				children.erase( children.begin( ) + i );

			children[i].draw_elements( i );
		}
	}
}