; ModuleID = 'awaithe.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.coro = type { %"struct.std::coroutine_handle" }
%"struct.std::coroutine_handle" = type { %"struct.std::coroutine_handle.0" }
%"struct.std::coroutine_handle.0" = type { i8* }
%_Z1gv.frame = type { void (%_Z1gv.frame*)*, void (%_Z1gv.frame*)*, i32, %"struct.coro::promise_type" }
%"struct.coro::promise_type" = type { %"struct.std::coroutine_handle.0" }
%_Z1fv.frame = type { void (%_Z1fv.frame*)*, void (%_Z1fv.frame*)*, i32, %"struct.coro::promise_type" }
%any.frame.1 = type { void (i8*)*, void (i8*)*, i32, i32 }

@.str = private unnamed_addr constant [17 x i8] c"alloc %zu => %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"delete %p\0A\00", align 1
@.str.5 = private unnamed_addr constant [12 x i8] c"post-resume\00", align 1
@.str.6 = private unnamed_addr constant [24 x i8] c"has waiter, resuming...\00", align 1
@.str.7 = private unnamed_addr constant [11 x i8] c"no waiter\0A\00", align 1
@str = private unnamed_addr constant [33 x i8] c"g running, just about to suspend\00"
@str.8 = private unnamed_addr constant [10 x i8] c"f started\00"
@str.9 = private unnamed_addr constant [10 x i8] c"f resumed\00"

; Function Attrs: nobuiltin nounwind uwtable
define noalias i8* @_Znwm(i64 %sz) #0 {
entry:
  %call = tail call noalias i8* @malloc(i64 %sz) #3
  %call1 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), i64 %sz, i8* %call)
  ret i8* %call
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #1

; Function Attrs: nounwind
declare i32 @printf(i8* nocapture readonly, ...) #1

; Function Attrs: nobuiltin nounwind uwtable
define void @_ZdlPv(i8* %p) #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
invoke.cont:
  %call = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), i8* %p)
  tail call void @free(i8* %p) #3
  ret void
}

declare i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind
declare void @free(i8* nocapture) #1

; Function Attrs: nounwind uwtable
define void @_Z6EscapeR4coro(%struct.coro* dereferenceable(8) %g) #2 {
entry:
  %0 = bitcast %struct.coro* %g to i8*
  tail call void asm sideeffect "", "imr,~{memory},~{dirflag},~{fpsr},~{flags}"(i8* %0) #3, !srcloc !1
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z1gv(%struct.coro* noalias nocapture sret %agg.result) #2 personality i32 (...)* @__gxx_personality_v0 {
entry:
  %0 = icmp eq i8* null, null
  br i1 %0, label %coro.alloc, label %return

coro.alloc:                                       ; preds = %entry
  %call.i = tail call noalias i8* @malloc(i64 32) #3
  %call1.i = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), i64 32, i8* %call.i) #3
  br label %return

return:                                           ; preds = %entry, %coro.alloc
  %1 = phi i8* [ null, %entry ], [ %call.i, %coro.alloc ]
  %2 = bitcast i8* %1 to void (%_Z1gv.frame*)**
  store void (%_Z1gv.frame*)* @_Z1gv.resume, void (%_Z1gv.frame*)** %2, align 8
  %3 = getelementptr i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to void (%_Z1gv.frame*)**
  %5 = select i1 %0, void (%_Z1gv.frame*)* @_Z1gv.destroy, void (%_Z1gv.frame*)* @_Z1gv.cleanup
  store void (%_Z1gv.frame*)* %5, void (%_Z1gv.frame*)** %4, align 8
  %__promise12 = getelementptr i8, i8* %1, i64 24
  %ptr.i.i = bitcast i8* %__promise12 to i8**
  store i8* null, i8** %ptr.i.i, align 8, !tbaa !2
  %6 = getelementptr inbounds %struct.coro, %struct.coro* %agg.result, i64 0, i32 0, i32 0, i32 0
  store i8* %1, i8** %6, align 8, !alias.scope !7
  %7 = getelementptr i8, i8* %1, i64 16
  %8 = bitcast i8* %7 to i32*
  store i32 1, i32* %8, align 4
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.coro.elide() #3

; Function Attrs: nounwind
declare i8* @llvm.coro.init(i8*) #3

; Function Attrs: nounwind uwtable
define void @_Z1fv(%struct.coro* noalias nocapture sret %agg.result) #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
entry:
  %0 = icmp eq i8* null, null
  br i1 %0, label %coro.alloc, label %return

coro.alloc:                                       ; preds = %entry
  %call.i = tail call noalias i8* @malloc(i64 32) #3
  %call1.i = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([17 x i8], [17 x i8]* @.str, i64 0, i64 0), i64 32, i8* %call.i) #3
  br label %return

return:                                           ; preds = %entry, %coro.alloc
  %1 = phi i8* [ null, %entry ], [ %call.i, %coro.alloc ]
  %2 = bitcast i8* %1 to void (%_Z1fv.frame*)**
  store void (%_Z1fv.frame*)* @_Z1fv.resume, void (%_Z1fv.frame*)** %2, align 8
  %3 = getelementptr i8, i8* %1, i64 8
  %4 = bitcast i8* %3 to void (%_Z1fv.frame*)**
  %5 = select i1 %0, void (%_Z1fv.frame*)* @_Z1fv.destroy, void (%_Z1fv.frame*)* @_Z1fv.cleanup
  store void (%_Z1fv.frame*)* %5, void (%_Z1fv.frame*)** %4, align 8
  %__promise30 = getelementptr i8, i8* %1, i64 24
  %ptr.i.i = bitcast i8* %__promise30 to i8**
  store i8* null, i8** %ptr.i.i, align 8, !tbaa !2
  %6 = getelementptr inbounds %struct.coro, %struct.coro* %agg.result, i64 0, i32 0, i32 0, i32 0
  store i8* %1, i8** %6, align 8, !alias.scope !10
  %7 = getelementptr i8, i8* %1, i64 16
  %8 = bitcast i8* %7 to i32*
  store i32 1, i32* %8, align 4
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define i32 @main() #4 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
invoke.cont1:
  %elided.frame = alloca %_Z1fv.frame, align 16
  %0 = bitcast %_Z1fv.frame* %elided.frame to <2 x void (%_Z1fv.frame*)*>*
  store <2 x void (%_Z1fv.frame*)*> <void (%_Z1fv.frame*)* @_Z1fv.resume, void (%_Z1fv.frame*)* @_Z1fv.cleanup>, <2 x void (%_Z1fv.frame*)*>* %0, align 16, !noalias !13
  %ptr.i.i.i = getelementptr %_Z1fv.frame, %_Z1fv.frame* %elided.frame, i64 0, i32 3, i32 0, i32 0
  store i8* null, i8** %ptr.i.i.i, align 8, !tbaa !2, !noalias !13
  %1 = getelementptr %_Z1fv.frame, %_Z1fv.frame* %elided.frame, i64 0, i32 2
  store i32 1, i32* %1, align 16, !noalias !13
  call fastcc void @_Z1fv.resume(%_Z1fv.frame* %elided.frame)
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str.5, i64 0, i64 0))
  ret i32 0
}

; Function Attrs: nounwind
declare void @llvm.coro.resume(i8*) #3

; Function Attrs: nounwind
declare i32 @puts(i8* nocapture readonly) #1

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1fv.resume(%_Z1fv.frame* nonnull %frame.ptr.resume) #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
resume.entry:
  %0 = getelementptr %_Z1fv.frame, %_Z1fv.frame* %frame.ptr.resume, i64 0, i32 2
  %index = load i32, i32* %0, align 4
  %switch = icmp eq i32 %index, 1
  br i1 %switch, label %_Z1gv.resume.exit, label %cleanup

_Z1gv.resume.exit:                                ; preds = %resume.entry
  %1 = bitcast %_Z1fv.frame* %frame.ptr.resume to i8*
  %puts = tail call i32 @puts(i8* nonnull getelementptr inbounds ([10 x i8], [10 x i8]* @str.8, i64 0, i64 0))
  store i32 2, i32* %0, align 4
  %puts.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([33 x i8], [33 x i8]* @str, i64 0, i64 0)) #3
  %call2.i.i.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.6, i64 0, i64 0)) #3
  %2 = bitcast i8* %1 to %any.frame.1*
  %3 = getelementptr %any.frame.1, %any.frame.1* %2, i32 0, i32 0
  %4 = load void (i8*)*, void (i8*)** %3
  call fastcc void %4(i8* %1)
  br label %exit

cleanup:                                          ; preds = %resume.entry
  %puts27 = tail call i32 @puts(i8* nonnull getelementptr inbounds ([10 x i8], [10 x i8]* @str.9, i64 0, i64 0))
  store i32 0, i32* %0, align 4
  %5 = getelementptr %_Z1fv.frame, %_Z1fv.frame* %frame.ptr.resume, i64 0, i32 3, i32 0, i32 0
  %6 = load i8*, i8** %5, align 8, !tbaa !2
  %tobool.i.i.i = icmp eq i8* %6, null
  br i1 %tobool.i.i.i, label %if.else.i.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %cleanup
  %call2.i.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.6, i64 0, i64 0)) #3
  %7 = load i8*, i8** %5, align 8, !tbaa !2
  %8 = bitcast i8* %7 to %any.frame.1*
  %9 = getelementptr %any.frame.1, %any.frame.1* %8, i32 0, i32 0
  %10 = load void (i8*)*, void (i8*)** %9
  call fastcc void %10(i8* %7)
  br label %exit

if.else.i.i:                                      ; preds = %cleanup
  %call5.i.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([11 x i8], [11 x i8]* @.str.7, i64 0, i64 0)) #3
  br label %exit

exit:                                             ; preds = %if.then.i.i, %if.else.i.i, %_Z1gv.resume.exit
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1fv.destroy(%_Z1fv.frame* nonnull %frame.ptr.destroy) #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
destroy.entry:
  %0 = bitcast %_Z1fv.frame* %frame.ptr.destroy to i8*
  %call.i18 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), %_Z1fv.frame* nonnull %frame.ptr.destroy) #3
  tail call void @free(i8* %0) #3
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1fv.cleanup(%_Z1fv.frame* nonnull %frame.ptr.cleanup) #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
destroy.entry:
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1gv.resume(%_Z1gv.frame* nonnull %frame.ptr.resume) #2 personality i32 (...)* @__gxx_personality_v0 {
init.ready:
  %puts = tail call i32 @puts(i8* getelementptr inbounds ([33 x i8], [33 x i8]* @str, i64 0, i64 0))
  %0 = getelementptr %_Z1gv.frame, %_Z1gv.frame* %frame.ptr.resume, i64 0, i32 2
  store i32 0, i32* %0, align 4
  %ptr.i.i.i = getelementptr %_Z1gv.frame, %_Z1gv.frame* %frame.ptr.resume, i64 0, i32 3, i32 0, i32 0
  %1 = load i8*, i8** %ptr.i.i.i, align 8, !tbaa !2
  %tobool.i.i.i = icmp eq i8* %1, null
  br i1 %tobool.i.i.i, label %if.else.i.i, label %if.then.i.i

if.then.i.i:                                      ; preds = %init.ready
  %call2.i.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([24 x i8], [24 x i8]* @.str.6, i64 0, i64 0)) #3
  %2 = load i8*, i8** %ptr.i.i.i, align 8, !tbaa !2
  %3 = bitcast i8* %2 to %any.frame.1*
  %4 = getelementptr %any.frame.1, %any.frame.1* %3, i32 0, i32 0
  %5 = load void (i8*)*, void (i8*)** %4
  call fastcc void %5(i8* %2)
  br label %exit

if.else.i.i:                                      ; preds = %init.ready
  %call5.i.i = tail call i32 @puts(i8* nonnull getelementptr inbounds ([11 x i8], [11 x i8]* @.str.7, i64 0, i64 0)) #3
  br label %exit

exit:                                             ; preds = %if.then.i.i, %if.else.i.i
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1gv.destroy(%_Z1gv.frame* nonnull %frame.ptr.destroy) #2 personality i32 (...)* @__gxx_personality_v0 {
exit:
  %0 = bitcast %_Z1gv.frame* %frame.ptr.destroy to i8*
  %call.i10 = tail call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), %_Z1gv.frame* nonnull %frame.ptr.destroy) #3
  tail call void @free(i8* %0) #3
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1gv.cleanup(%_Z1gv.frame* nonnull %frame.ptr.cleanup) #2 personality i32 (...)* @__gxx_personality_v0 {
exit:
  ret void
}

attributes #0 = { nobuiltin nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (https://github.com/GorNishanov/clang.git 7007ac36c6072caf31bb9bf95a3aa13361c7b5c7) (https://github.com/GorNishanov/llvm.git b9c40a2bc94c548377501092d0438371e9e93327)"}
!1 = !{i32 1248}
!2 = !{!3, !4, i64 0}
!3 = !{!"_ZTSSt16coroutine_handleIvE", !4, i64 0}
!4 = !{!"any pointer", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8}
!8 = distinct !{!8, !9, !"_ZN4coro12promise_type17get_return_objectEv: %agg.result"}
!9 = distinct !{!9, !"_ZN4coro12promise_type17get_return_objectEv"}
!10 = !{!11}
!11 = distinct !{!11, !12, !"_ZN4coro12promise_type17get_return_objectEv: %agg.result"}
!12 = distinct !{!12, !"_ZN4coro12promise_type17get_return_objectEv"}
!13 = !{!14}
!14 = distinct !{!14, !15, !"_Z1fv: %agg.result"}
!15 = distinct !{!15, !"_Z1fv"}
