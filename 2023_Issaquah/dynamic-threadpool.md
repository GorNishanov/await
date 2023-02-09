# Thoughts on implementing dynamic threadpool on Linux

For the purpose of this paper:

* Fixed size threadpool
  - A threadpool that has N number of threads that does not grow or shrink as workload changes.
* Dynamic threadpool
  - A threadpool that can grow and shrink

Both Darwin/XNU and Windows have user-mode dynamic sized threadpool that
 get a kernel help in deciding how many threads should be active to support
     the workload managed by the user mode part of the threadpool.

On Windows, for example, there is a completion port that limit how many threads can be actively pulling from it based on the knowledge whether the threads that are working on the dequeued items are actively computing something or blocked waiting on some kernel resource. Another facility that is responsible for creating a new threads user mode threadpool would slow down thread creation if it notices that threads are getting put in a wait state within some window.
