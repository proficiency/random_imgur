#pragma once
#include <WinInet.h>
#include <atlbase.h>
#include <random>

// endlessly inefficent
class c_networking
{
public:
	// find a random image and download it
	u8a download_image( )
	{
		std::wstring  url  = resolve_random_url( );
		u8a			  ret  = download_image( url );

		// hash = image has been removed
		while ( fnv32::hash( ret.data( ), ret.size( ) ) == 0x3035f080 )
		{
			url = resolve_random_url( );
			ret = download_image( url );
		}

		ctx.m_previous_urls.push_back( url );
		return ret;
	}

	// download image from url
	u8a download_image( const std::wstring& url )
	{
		std::unique_ptr< char >  buffer     = std::make_unique< char >( );
		u8a             		 data		= {};
		u32						 data_size	= 0;
		CComPtr< IStream >		 stream     = {};

		if ( CoInitialize( nullptr ) < 0 )
			return {};

		if ( URLOpenBlockingStreamW( nullptr, url.c_str( ), &stream, 0, 0 ) < 0 )
			return {};		

		while ( true )
		{
			unsigned long bytes_read;
			if ( stream->Read( buffer.get( ), sizeof( buffer ), &bytes_read ) == S_FALSE )
				break;

			if ( bytes_read == 0 )
				continue;

			data.insert( data.end( ), buffer.get( ), buffer.get( ) + sizeof( buffer ) );
			data_size += bytes_read;
		}

		data.erase( data.begin( ) + data_size, data.end( ) );
		return data;
	}

private:
	// you could just follow the redirect from imgur.com/random/, add i. to the domain, remove /gallery, and append .jpg
	std::wstring resolve_random_url( )
	{
		static const std::wstring  alphabet  = L"abcdefghijklmnopqrstuvwxyz0123456789";
		std::wstring			   url		 = {};

		std::random_device					  device;
		std::mt19937						  rng( device( ) );
		std::uniform_int_distribution< u32 >  range( 0, alphabet.size( ) - 1 );

		for ( u32 i = 0; i < 5; ++i )
		{
			char entry = alphabet[range( rng )];

			if ( std::isdigit( entry ) )
				url.push_back( entry );			

			else
				url.push_back( range( rng ) % 2 == 0 ? towupper( entry ) : entry );			
		}

		return std::wstring( L"https://i.imgur.com/" ) + url + L".jpg";
	}
}; inline c_networking g_networking;