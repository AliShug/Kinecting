## Kinecting
Prototyping with the Kinect V2

### Overview

**KinectDevice**: Wraps up the Kinect API, providing an asynchronous depth stream. See `kinecting.cpp#main()` for example use.

**GLWindow**: Window managing an OpenGL context, containing a *GLScene* composed of one or more *GLObjects*. Note that objects can't currently be shared between windows!

**GLObject**: Manages mesh data for a 3D object. Meshes can be generated with any one of several functions, and each object can be displayed as a point cloud, line strip, line collection or triangle mesh. Vertex data includes position, colour, UV coordinates and normals - shaders can use or discard these components as necessary.

**Texture**: Manages an OpenGL texture, including image upload from CPU->GPU. Wraps up the OpenGL format flags, and can be resized as needed.

**ShaderManager**: Loads, compiles and manages an OpenGL shader program comprised of a vertex and fragment shader. Each object has an attached shader manager and corresponding set of shaders. The manager parses shader parameters and uniforms, making binding data and textures to the shaders much simpler.

**NormDepthImage**: Depth image processing platform. Generates normals based on an input depth image, and provides filtering, flood-fill masking and masked smoothing operations.

**PointCloud**: Manages and provides basic processing functions for point-cloud data.

### Usage
