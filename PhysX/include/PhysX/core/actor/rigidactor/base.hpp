#ifndef PX_CONTENT_ACTOR_RIGIDACTOR_BASE_HPP
#define PX_CONTENT_ACTOR_RIGIDACTOR_BASE_HPP

#include <PxPhysicsAPI.h>

#include <Nebula/Actor/RigidActor/Base.hh>

#include <PhysX/core/actor/actor/base.hpp>
#include <PhysX/core/actor/util/decl.hpp>

namespace phx { namespace core { namespace actor { namespace rigidactor {



	class base:
		virtual public neb::core::actor::rigidactor::base,
		virtual public phx::core::actor::actor::base
	{

		public:
			base(sp::shared_ptr<phx::core::actor::util::parent>);

			virtual void					init();
			virtual void					release();
			virtual void					step(neb::core::TimeStep const & ts);

			void						setupFiltering();

			virtual void					create_physics() = 0;
			virtual void					init_physics() = 0;

			virtual void					add_force(real) = 0;
			//virtual void					set_pose(physx::PxTransform);


			virtual sp::shared_ptr<phx::core::actor::rigiddynamic::local>		get_projectile() = 0;


			physx::PxActor*					px_actor_;

			//std::shared_ptr<neb::core::actor>		object_;
	};


}}}}

#endif


