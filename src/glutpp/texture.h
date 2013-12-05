#ifndef __GRU_TEXTURE_H__
#define __GRU_TEXTURE_H__

#include <functional>

#include <GL/glew.h>

#include <math/mat44.h>
#include <math/vec4.h>
#include <math/vec3.h>

#include <glutpp/uniform.h>

namespace glutpp
{
	class window;
	class texture
	{
		public:
			texture(window*);
			~texture();
			void			init_shadow(int,int);
			int			load_png(char const *);
			void			bind();

			GLint			w_;
			GLint			h_;
			GLuint			o_;

			window*			window_;
	};
}

#endif








