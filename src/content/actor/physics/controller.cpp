#include <nebula/define.hpp>

#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include <jess/free.hpp>
#include <jess/ostream.hpp>

#include <nebula/utilities/free.hpp>
#include <nebula/content/actor/admin/controller.hpp>
#include <nebula/content/actor/physics/controller.hpp>

nca::physics::controller::controller()
{
	jess::clog << NEB_FUNCSIG << std::endl;

}
nca::physics::controller::~controller()
{
	jess::clog << NEB_FUNCSIG << std::endl;

}
void	nca::physics::controller::init( jess::shared_ptr<ncaa::base> parent )
{
	jess::clog << NEB_FUNCSIG << std::endl;
	
	ncap::base::init( parent );
}
void	nca::physics::controller::shutdown()
{
	jess::clog << NEB_FUNCSIG << std::endl;

}
void	nca::physics::controller::update()
{
	jess::clog << NEB_FUNCSIG << std::endl;

}
void	nca::physics::controller::step(FLOAT dt)
{
	jess::clog << NEB_FUNCSIG << std::endl;

	update_move();
	
	jess::shared_ptr<ncaa::controller> parent = std::dynamic_pointer_cast<nca::admin::controller>(parent_.lock());
	
	parent->move_ = prod( nebula::utilities::matrix_yaw( parent->yaw_ ), parent->move_ );

	jess::clog << "move_=" << parent->move_ << std::endl;
	
	parent->pos_ += parent->move_ * dt;

	jess::assertion( parent->velocity_.size() == 3 );

	// gravity for a flat world
	float y_eye = 1.0;
	float v = parent->velocity_(1);
	float y = parent->pos_(1) - y_eye;
	
	if( y > 0 )
	{
		v -= 9.81 * dt;
		
		y += v * dt;
	
		if( y <= 0 )
		{
			y = 0;
			v = 0;
		}
	}
	
	parent->velocity_(1) = v;
	parent->pos_(1) = y + y_eye;

}
void	nca::physics::controller::update_move()
{
	// log
	jess::clog << NEB_FUNCSIG << std::endl;

	jess::shared_ptr<ncaa::controller> parent = std::dynamic_pointer_cast<nca::admin::controller>(parent_.lock());

	

	/** \todo
	 * add gravity
	 * make \a head and \a m a static member variable or at least a member variables
	 */

	// the following scheme provides equal magnitude for each direction and uniformly spaced directions (i.e. diagonal is at exactly 45 degrees)
	FLOAT s = 1;
	FLOAT d = 0.707;

	s *= 1.5;
	d *= 1.5;

	bnu::matrix<FLOAT> head(8,3);
	head(0,0) = 0;		head(0,1) = 0;		head(0,2) = -s;
	head(1,0) = d;		head(1,1) = 0;		head(1,2) = -d;
	head(2,0) = s;		head(2,1) = 0;		head(2,2) = 0;
	head(3,0) = d;		head(3,1) = 0;		head(3,2) = d;
	head(4,0) = 0;		head(4,1) = 0;		head(4,2) = s;
	head(5,0) = -d;		head(5,1) = 0;		head(5,2) = d;
	head(6,0) = -s;		head(6,1) = 0;		head(6,2) = 0;
	head(7,0) = -d;		head(7,1) = 0;		head(7,2) = -d;


	std::map<int,int> m;
	m[nca::admin::controller::flag::eNORTH							] = 0;
	m[nca::admin::controller::flag::eNORTH	|	nca::admin::controller::flag::eEAST	] = 1;
	m[						nca::admin::controller::flag::eEAST	] = 2;
	m[nca::admin::controller::flag::eSOUTH	|	nca::admin::controller::flag::eEAST	] = 3;
	m[nca::admin::controller::flag::eSOUTH							] = 4;
	m[nca::admin::controller::flag::eSOUTH	|	nca::admin::controller::flag::eWEST	] = 5;
	m[						nca::admin::controller::flag::eWEST	] = 6;
	m[nca::admin::controller::flag::eNORTH	|	nca::admin::controller::flag::eWEST	] = 7;

	// ignore all other flags
	int f = parent->flag_ & ( nca::admin::controller::flag::eNORTH |
			nca::admin::controller::flag::eSOUTH | nca::admin::controller::flag::eEAST |
			nca::admin::controller::flag::eWEST );

	// find vector for move flag
	auto it = m.find( f );

	if ( it != m.end() )
	{
		parent->move_ = bnu::matrix_row< bnu::matrix<FLOAT> >(head,it->second);


		jess::cout << "move_=" << parent->move_ << std::endl;
	}
	else
	{
		parent->move_ = bnu::zero_vector<FLOAT>(3);

		jess::cout << "move_=" << parent->move_ << std::endl;
	}
}








