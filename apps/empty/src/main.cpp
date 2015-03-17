

/*

#include <glm/gtc/matrix_transform.hpp>

#include <gal/console/base.hpp>

#include <gal/etc/stopwatch.hpp>

#include <neb/fnd/free.hpp>
#include <neb/fnd/util/cast.hpp>
#include <neb/fnd/core/actor/Base.hpp>
#include <neb/fnd/util/wrapper.hpp>
#include <neb/fnd/core/light/base.hpp>
#include <neb/fnd/core/scene/Base.hpp>
#include <neb/fnd/core/shape/base.hpp>
#include <neb/fnd/core/shape/cuboid/desc.hpp>
#include <neb/fnd/core/actor/Base.hpp>
#include <neb/fnd/core/actor/rigidbody/desc.hpp>
#include <neb/fnd/game/map/base.hpp>
#include <neb/fnd/game/trigger/ActorEx1.hpp>
#include <neb/fnd/game/game/desc.hpp>
#include <neb/fnd/game/weapon/SimpleProjectile.hpp>
#include <neb/fnd/context/Window.hpp>
#include <neb/fnd/environ/Two.hpp>
*/

#include <neb/fnd1/app/Base00.hpp>

int			main(int ac, char ** av)
{
	auto app = neb::fnd1::app::Base00::s_init(ac, av);
	
//	int p = atoi(av[1]);
	
//	app->create_server(p);
	
	app->loop();
}



