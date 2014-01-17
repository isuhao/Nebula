#ifndef __NEBULA_CONTENT_ACTOR_PHYSICS_BASE_HPP__
#define __NEBULA_CONTENT_ACTOR_PHYSICS_BASE_HPP__

#include <memory>


//#include <tinyxml/tinyxml.h>

#include <math/transform.h>

#include <glutpp/actor/actor.h>

#include <PxPhysicsAPI.h>

#include <glutpp/actor/desc.h>

#include <neb/config.h>

namespace neb
{
	namespace actor
	{
		class Base: public glutpp::actor::actor
		{
			public:
				Base(
						std::shared_ptr<neb::scene::scene>,
						neb::actor::Base_s = neb::actor::Base_s());
				virtual ~Base();

				virtual void			init(glutpp::actor::desc_s);
				virtual void			release();
				
			private:
				neb::actor::Base_s		create_actor(
						glutpp::actor::desc_s);
			public:
				neb::actor::Base_s		create_actor_local(
						glutpp::actor::desc_s);
				
				neb::actor::Base_s		create_actor_remote(
						glutpp::actor::addr_s,
						glutpp::actor::desc_s);
				
				
				void				create_shapes(glutpp::actor::desc_s);
				void				create_children(glutpp::actor::desc_s);
				
				virtual void			create_physics() {abort();}
				virtual void			init_physics() {abort();}

				neb::app_s			get_app();
				neb::scene::scene_s		get_scene();
				neb::actor::Base_s		get_actor(int);
				neb::actor::Base_s		get_actor(glutpp::actor::addr_s);

				virtual glutpp::actor::desc_s	get_projectile();



				virtual void			set_pose(math::transform);
				virtual int			fire();
				
				virtual void			step(double) { abort(); }
				virtual void			step_remote(double);

				// signal
				void				connect(glutpp::window::window_s);
				
				int				key_fun(int,int,int,int);
				
				
				// conversion
				rigid_body::rigid_body_s	to_rigid_body();
				
			public:
				glutpp::window::window_w	window_;
				
				struct
				{
					key_fun_c		key_fun_;
				} conn_;

		};
	}
}


#endif








