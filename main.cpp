#include "context.h"

int main( )
{
	g_render.add_callback( ui::draw, c_render::CONTEXT_DRAW );
	g_render.setup( );
	return 0;
}