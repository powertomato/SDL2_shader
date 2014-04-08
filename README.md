About
=====
SDL2_shader is an inofficial SDL2 extension library intended to allow the
use of shaders with a number of SDL2 rendering backends. Currently opengl
opengles2 and direct3d are supported and direct3d11 (WinRT) is in
development.

License
=======
SDL2_shader is developed under a liberal zlib license. For details see
[this wikipedia article](http://en.wikipedia.org/wiki/Zlib_License) or LICENSE
file.

Usage
=====
SDL2_shader is in early alpha stage. For now it shouldn't be used in production
environments if you don't want to get your hands dirty: Testcases and reference
renderings are in the repository and need to be tested on as many plattforms
and by as many users as possible.

The best option for now is droping the whole codebase in your project, since
there are no official builds. There are a few macros which to choose the 
desired backends:
 * SDL_SHADER_D3D - enables the direct3d backend
 * SDL_SHADER_D3D11 - enables the direct3d backend (not yet)
 * SDL_SHADER_OPENGL - enables the opengl backend
 * SDL_SHADER_OPENGLES2 - enables the opengles2 backend

Code sample
-----------

Using SDL_Shader is very easy:
~~~C
SDL_Shader *shader = SDL_createShader( renderer, "path/to/shader" );
...
SDL_renderCopyShd( shader, textures[i], NULL, &dst );
~~~

The above code code compiles and assembles the shaders. Depending on the 
concrete backend it looks for the following:
 * shader.d3d.hlsl - for the direct3d shaders
 * shader.gles2.frag - for the opengles2 fragment shader
 * shader.gles2.vert - for the opengles2 vertex shader
 * shader.gl.frag - for the opengles2 fragment shader
 * shader.gl.vert - for the opengl vertex shader

This is the bad news: for every backend you want to support, you have to port
your shader to. You leave the nice world of SDL where everything is abstracted
away. There are several solutions to this problem, SDL2_shader is not (yet?)
one of them.

Writing shaders
--------------
If you write shaders you will be restricted a little bit. You have to use
predetermined variable names and texture lookups may need color conversion.
Take a look the "do_nothing" shader from the test-directory as a template.

Contributing
============
The easiest way to contribute is by forking the repo on
[github](https://github.com/powertomato/sdl2_shader) make some changes and make
a pull request. If you don't have (or don't want) a github accout you also may
to drop me an email with a patch file or instructions. If you choose the latter
also don't forget to write which (nick)name you want to show up in the 
repository.

Anyway please make sure not to follow these guidelines to make maintaining
a bit easier:

 * Don't do any unnecessary changes (e.g. fix typos in unrelated files)
 * Don't change the interface i.e. change the way the library is used
 * Keep your code K&R-ish
 * Use tabs for identiation (spaces may be used in comments), assume a tab-
   length of 4

Dev. Roadmap
============

If you want to contribute here is the list of TODOs which has to be done. A *
denotes the feature is beeing worked on.

High priority
-------------
 * direct3d11 backend*
 * more testcases*
 * integrate an automatic testing framework
 * Decide on a definite interface

Lower priority
------------
 * integrate a build system like cmake or automake
 * official builds
 * code cleanup
 * a 2D-lightning shader like [this](https://www.youtube.com/watch?v=Xmn6zhDJGLE)
 or [this](http://www.catalinzima.com/2010/07/my-technique-for-the-shader-based-dynamic-2d-shadows/)

Nice to have
------------
 * more backends
 * IDEs projects
 * Common shader language parser

