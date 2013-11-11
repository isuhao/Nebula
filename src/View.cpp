
#include <assert.h>

#include <GRU/Window.h>

#include <NEB/Scene.h>
#include <NEB/View.h>

NEB::View::View():
	scene_( NULL )
{

}
void	NEB::View::SetWindow( GRU::Window* window )
{
	assert( window );
	
	window->CallBackDisplay_ = std::bind( &NEB::View::Display, this );
}
void	NEB::View::Display()
{
	if( scene_ )
	{
		scene_->Display();
	}
}




