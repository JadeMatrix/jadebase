#Introduction to `jadebase`

**jadebase** is a spin-off of the [BQTDraw](https://github.com/JadeMatrix/BQTDraw) project.  During the development of BQTDraw, it became clear that much of the development effort was being put into various generic utilities and wrapping platform-specific code rather than developing the actual application.  jadebase was split of the BQTDraw codebase in an effort to provide these utilities as a standalone library for use in a wide variety of professional software applications.

Central to jadebase's design is the concept of tasks.  Many legacy codebases for game engines, art software, and other such applications do not utilize multithreading to its full potential, if at all.  In an effort to help rectify this, the core of jadebase is a pool of worker threads.  'Tasks' are logically or procedurally discrete pieces of code that are distributed amongst these workers for execution.  Essentially all application and framework code are tasks, allowing the application to automatically scale out to utilize the full threading capabilities of the host CPU.

###Further Reading

[The jadebase Task Framework](Tasking.md) (in detail)

[jadebase's Input Event Model](Events.md)

[The jadebase GUI Framework](GUI.md)
