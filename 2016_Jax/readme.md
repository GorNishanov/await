# Coroutines on Linux demo

We forked clang 3.8.0 to experiment with various way of representing coroutines in llvm IR and testing different approaches of integrating coroutine optimization and implementation passes in the optimizer pipeline. We are on the fourth iteration and based on the discussion this week we will need to iterate on it more before cleaning up the code and bringing to llvmdev for formal review.

Result of the experiment demonstrate heap elision and indirect call replacement for two scenarios, simple generators and asynchronous nested coroutines. clang work is an extension of Richard Smith coroutine commits. LLVM work is based on brainstorming with Richard and David, but likely misinterpreted and hastily implemented by me.

## Demo coroutines and resulting post optimized IR

Simple generator demo:
````
generator<int> f() {
  int N = 200;
  for (int i = 3; i < N; i *= 2)
    co_yield i;
}

int main() {
  auto g = f();
  // Escape(g); // uncomment to disable heap elision for g
  for (auto v : g)
    printf("%d\n", v);
}
````
Compiling and running it on linux produces an excepted result which is not particularly impressive: 

````
./clang -Xclang -fcoroutines -O2 -std=c++14 gen.cpp -S -emit-llvm && ./a.out
3
6
12
24
48
96
192
```` 
However compiling it to show the disassembly shows that coroutine disappeared completely, the body of main consists of just printf statements with the constants 3 .. 192.
````
./clang -Xclang -fcoroutines -O2 -std=c++14 gen.cpp -S -emit-llvm
cat gen.ll
...
; Function Attrs: norecurse nounwind uwtable
define i32 @main() #4 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
invoke.cont7.lr.ph:
  %call8 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 3)
  %call8.1 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 6)
  %call8.2 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 12)
  %call8.3 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 24)
  %call8.4 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 48)
  %call8.5 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 96)
  %call8.6 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([4 x i8], [4 x i8]* @.str.2, i64 0, i64 0), i32 192)
  ret i32 0
}
...
````

## Building your own clang/llvm

Follow instructions from: http://clang.llvm.org/get_started.htm and pull from 

