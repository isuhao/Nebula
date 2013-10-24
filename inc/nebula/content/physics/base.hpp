#ifndef _CONTENT_PHYSICS_PHYSICS_H
#define _CONTENT_PHYSICS_PHYSICS_H

#include <nebula/define.hpp>
#include <PxPhysicsAPI.h>

#include <memory>

#include <jess/shared_ptr.hpp>

#include <nebula/ns.hpp>

/// default error callback for %PhysX
class DefaultErrorCallback: public physx::PxErrorCallback
{
	public:
		/// Reports an error code
		void 	reportError( physx::PxErrorCode::Enum code, const char *message, const char *file, int line);
};

namespace nebula
{
	namespace content
	{
		namespace physics
		{
			/// physics for app
			/*
			 * -custom fun30000tionality
			 *     -drag-lift
			 *
			 * -before buffer swap
			 *     -onTrigger
			 *     -onContact
			 *     -onConstraintBreak
			 * -after buffer swap
			 *     -onSleep
			 *     -onWake
			 * -callbacks that occur for all objects
			 *     -onConstraintBreak
			 *     -onSleep
			 *     -onWake
			 * -callbacks that only occur if filter shader callback flag is set
			 *     -onTrigger
			 *     -onContact
			 */
			class base
			{
				public:
					/// ctor
					base( jess::shared_ptr<n30000::base> );
					/// dtor
					virtual ~base();
					/// init
					virtual	void							init();
					/// shutdown
					virtual	void							shutdown();
					///@name create
					///@{
					/// scene
					jess::shared_ptr<n32200::base>					create_scene();
					/// rigid dynamic
					jess::shared_ptr<n34200::rigid_dynamic>				create_rigid_dynamic(
							jess::shared_ptr<n32100::base>,
							jess::shared_ptr<n34100::base>
							);
					/// rigid dynamic box
					jess::shared_ptr<n34200::rigid_dynamic_box>			create_rigid_dynamic_box(
							jess::shared_ptr<n32100::base>,
							jess::shared_ptr<n34100::base>
							);
					/// controller
					jess::shared_ptr<n34200::controller>				create_controller(
							jess::shared_ptr<n32100::base>,
							jess::shared_ptr<n34100::base>
							);
					/** box
					 */
					jess::shared_ptr<n35200::box>					create_box( jess::shared_ptr<n35100::box> );
					/** box
					 */
					jess::shared_ptr<n35200::plane>					create_plane( jess::shared_ptr<n35100::plane> );

					/// physics material
					jess::shared_ptr<n34200::material>				request_physics_material();
					/// physics material
					jess::shared_ptr<n34200::material>				create_physics_material();
					///@}
					void								init_rigid_actor( jess::shared_ptr<n34200::rigid_actor> actor );
					///@name fetch
					///@{
					/// physx physics
					physx::PxPhysics*						get_px_physics() const
					{
						return px_physics_;
					}
					/// physx controller manager
					physx::PxControllerManager*					get_px_controller_manager() const
					{
						return px_character_controller_manager_;
					}
					///@}
				protected:
					/// parent
					std::weak_ptr<n30000::base>				parent_;
					/// default material
					jess::shared_ptr<n34200::material>				default_material_;
					///@name physx
					///@{
					/**
					 */
					/// physx default error callback
					DefaultErrorCallback 						px_default_error_callback_;
					/// physx default allocator
					physx::PxDefaultAllocator 					px_default_allocator_callback_;
					/// physx foundation
					physx::PxFoundation*						px_foundation_;
					/// physx physics
					physx::PxPhysics*						px_physics_;
					/// physx profile zone manager
					physx::PxProfileZoneManager*					px_profile_zone_manager_;
					/// physx cooking
					physx::PxCooking*						px_cooking_;
					/// physx cuda context manager
					physx::pxtask::CudaContextManager*				px_cuda_context_manager_;
					/// physx controller manager
					physx::PxControllerManager*					px_character_controller_manager_;
					///@}
			};
		}
	}
}

#endif

