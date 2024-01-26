# InstrumentationCallbacks
This library extends the functionality of instrumentation callbacks, an undocumented Windows debugging feature. Instrumentation callbacks can be used to intercept kernel-to-user transitions for a process from user-mode. 

## Features
Currently, the library can be used to intercept the following:

+ System calls
+ User-mode APCs
+ User-mode exceptions
+ User-mode kernel callbacks (such as those used by WIN32K)
+ Initialization of new user-mode threads

The library's only dependency is NTDLL, so it can be run natively. It is currently only compatible with x86-64 (AMD64).

## Credits
+ [instrumentation_callbacks](https://github.com/Deputation/instrumentation_callbacks) ([Deputation](https://github.com/Deputation))
+ [phnt](https://github.com/winsiderss/systeminformer/tree/master/phnt) ([winsiderss](https://github.com/winsiderss))
