#pragma once
#include <time.h>

namespace util
{
	inline std::string unicode_to_ascii( const std::wstring& ws )
	{
		return std::string( ws.begin( ), ws.end( ) );
	}

	inline void to_clipboard( const std::string& s )
	{
		HGLOBAL alloc = GlobalAlloc( GMEM_MOVEABLE, s.size( ) + 1 );

		if ( alloc )
		{
			void* lock = GlobalLock( alloc );

			if ( lock )
			{
				memcpy( lock, s.c_str( ), s.size( ) + 1 );

				OpenClipboard( 0 );
				EmptyClipboard( );
				SetClipboardData( CF_TEXT, alloc );
				CloseClipboard( );

				GlobalUnlock( lock );
				GlobalFree( alloc );
			}
		}
	}
}