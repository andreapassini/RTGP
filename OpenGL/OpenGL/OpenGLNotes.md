# **Pipeline**
## **Vertex**
A piece of data containing the information of a specific point in space. It may have different attributes, but the most important one is the position.
## **Vertex Buffer** 
- A buffer to save every vertex.
- Declare unique vertices and store them in an array., in our example we called it positions []. A vertex buffer will allow us to specify the characteristics of the piece of geometry we want to render, for example by specifying the dimension.
## **Define Layout for the Vertex Buffer**
###	**Index Buffer**
For each piece of geometry, we want to render, we can store the index of the vertices composing it in an array, called Index Buffer. By doing so we reuse vertices and save a lot of memory, in a square we can save 2 vertices.
## **Bound** 
Bounding the vertex buffer to the vertex array buffer using ***glVertexAttribPointer***
##	**Vertex Array Buffer** 
-	If we use the **COMPATIBILITY PROFILE** one is created by default
-	IF we use the **CORE PROFILE**, we need to create at least one of them.



# **Shader**
A shader is a program written for the GPU. 
In the pipeline specified we write them on a file and, at the start of the main, we read them. In the file they are divided by #shader vertex and #shader fragment
We can have 2 different types of shaders:
-	**Vertex**
-	**Fragment**

In our example we associated vertex shader with the position and fragment shader with the color that we want to render.

## **Uniforms**
Piece of data that we send to the GPU that will be used for the shader. In our example we used them to change, in real time, the color of the square.



# **Abstraction**

Creating classes for handling more easily multiple objects to render.

Idealy we want to pick up a piece of geometry and call rendering function, specifying some parameters, and the renderer will take care of everything.

Following the example we will create:

- Vertex Buffer
- Index Buffer
- Renderer
- Vertex Array
    That will tight togheter vertex buffer with the layout specified.
    The layout might be on the GPU. but might be useful to keep it also in the CPU.
- Buffer Layout Object

