The code in this directory is under MIT license. See License.txt

This is sample code to go with the CppCon2015 presentation: C++ Coroutines: A negative overhead abstraction.

* PAL.* - platform abstraction layer
* Traditional.cpp - implementation of a state machine using callback machinery
* Improved.cpp - implementation of a state machine using callback machinery + sync completion + no allocations on the hot path
* Awaitable.cpp - implementation of a state machine via coroutine + awaitable machinery
* main.cpp - test driver

uncomment WHAT_TO_RUN line that correspond to the implementation you want to test
 
   sync = 0 => synchronous completions are handled via thread pool
   sync = 1 => synchronous completions are handled inline
   nReaders => how many client threads to launch
	
The code was tested using Visual Studio 2015 RTM.
Coroutines in VS2015 RTM do not support exceptions, hence the use of panic_if.
	- Do not use /ZI (Edit and Continue debugging). use /Zi (small i)