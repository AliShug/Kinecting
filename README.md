## Kinecting
Prototyping with the Kinect V2

### Overview

**KinectDevice**: Wraps up the Kinect API, providing an asynchronous depth stream. See `kinecting.cpp#main()` for example use.

**GLWindow**: Window managing an OpenGL context, containing a *GLScene* composed of one or more *GLObjects*. Note that objects can't currently be shared between windows!
**Texture**: Manages an OpenGL texture, including image upload from CPU->GPU. Wraps up the OpenGL format flags, and can be resized as needed.
**ShaderManager**: Loads, compiles and manages an OpenGL shader program comprised of a vertex and fragment shader. Each object has an attached shader manager and corresponding set of shaders. The manager parses shader parameters and uniforms, making binding data and textures to the shaders much simpler.


### Usage
