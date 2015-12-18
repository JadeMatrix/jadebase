
#jadebase
**jadebase** is a cross-platform framework for developing professional software applications in C++ and Lua.  It provides a task-based threading framework and a GUI system, as well as various utilities.

jadebase is developed by Joseph Durel [<jadematrix.art@gmail.com>] and licensed under the [zlib license](http://www.zlib.net/zlib_license.html), copyright 2014-2015.  It employs & supplies patches for [FastFormat](http://www.fastformat.org/) and [STLSoft](http://www.stlsoft.org/) by Matthew Wilson, which is licensed under a modified BSD license (included in the [LICENSE file](https://github.com/JadeMatrix/jadebase/blob/master/LICENSE) just to be safe).

#Current Features

##Threading
jadebase provides classes for creating & controlling threads, muteces, conditions, and semaphores in an object-oriented manner.  In addition, a `scoped_lock` template class is provided for simple & safe lock control.

##Tasking
jadebase is structured to be almost entirely task-based.  By inheriting from a base `jade::task` class, developers can create executable objects to pass to a central control system that dispatches to worker threads.

##GUI
Provides a Lua-scriptable GUI system with buttons, sliders, tabs, scrolling, groups, and more.

##Events
Unified, cross-platform-safe events, including tablet input

##Settings
Simple interface for storing & loading program settings from `.CFG` text files

#Upcoming Features

* **Change Tracking**  
Unified, opaque multiple undo & redo system
* **Dynamics**  
Combines value constraints & animation into a single generic system
* **System Menu Integration**  
Integration with each platform's menu bar and right-click menus

#Documentation

##Building & Installing

###Requirements
* STLSoft 1.9.124+
* FastFormat 0.7.1+
* Clang
* Lua (5.2 on Linux, 5.3 on OS X)[^LuaOnLinux]
* PangoCairo
* GLEW

###Shared Library & Headers
Currently only the Linux (GNU/Posix + X window system) port of jadebase builds.  To install the jadebase shared library, run `make linux_install`.  Being a C++ library, jadebase's [ABI](https://en.wikipedia.org/wiki/Application_binary_interface) can differ across compilers.  For future-proofing against support for multiple compilers, the shared library is named `libjadebase-${CPPC}.so` where CPPC is the C++ compiler named in the Makefile.  The shared library and appropriate symlinks are placed in `/usr/local/lib/`; header files are placed in `/usr/local/include/jadebase/`.

[^LuaOnLinux]: Lua version probably does not matter, but requirements are kind of hard-coded in the Makefile for now