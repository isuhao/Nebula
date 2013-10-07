#include <jess/ostream.hpp>

#include <nebula/define.hpp>
#include <nebula/content/shape/physics/base.hpp>
#include <nebula/content/shape/renderer/base.hpp>

#include <nebula/content/shape/admin/base.hpp>

nebula::content::shape::admin::base::base()
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

	scale_ = bnu::vector<float>( 3 );

	scale_(0) = 1;
	scale_(1) = 1;
	scale_(2) = 1;

}
nebula::content::shape::admin::base::~base()
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

}
void	nebula::content::shape::admin::base::init( jess::shared_ptr<nebula::content::actor::admin::rigid_actor> parent )
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

	parent_ = parent;

}
void	nebula::content::shape::admin::base::shutdown()
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;
}
void	nebula::content::shape::admin::base::render( jess::shared_ptr<nebula::platform::renderer::base> rnd )
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

	renderer_->render( rnd );
}
bnu::vector<float>	ncsa::base::get_scale()
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

	return scale_;
}




