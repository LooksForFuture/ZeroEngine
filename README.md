# ZeroEngine
Zero is a game engine/framework written for c++ with simplicity and functionality in mind.<br />
Zero has been designed to be both user friendly and configurable. The only hard requirements are opengl and glm.<br /><br />
Special thanks to @royvandam for his <a href="https://github.com/royvandam/rtti">rtti</a> library.

## Structure
The engine class controls all entities and their components. And the pipelines manage extra functionality such as rendering, audio, ohysics and etc.<br />
Currently only render and physics pipeline have been implemented.

## Quickstart
I currently use GLAD for loading opengl and GLFW for window management in my examples. But the engine doesn't strictly require them and you can use any other library you want.
