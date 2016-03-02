; ModuleID = 'gen.cpp'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.generator = type { %"struct.std::coroutine_handle" }
%"struct.std::coroutine_handle" = type { %"struct.std::coroutine_handle.0" }
%"struct.std::coroutine_handle.0" = type { i8* }
%_Z1fv.frame = type { void (%_Z1fv.frame*)*, void (%_Z1fv.frame*)*, i32, %"struct.generator<int>::promise_type", i32 }
%"struct.generator<int>::promise_type" = type { i32 }

@.str = private unnamed_addr constant [17 x i8] c"alloc %zu => %p\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"delete %p\0A\00", align 1
@.str.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

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
define void @_Z6EscapeR9generatorIiE(%struct.generator* dereferenceable(8) %g) #2 {
entry:
  %0 = bitcast %struct.generator* %g to i8*
  tail call void asm sideeffect "", "imr,~{memory},~{dirflag},~{fpsr},~{flags}"(i8* %0) #3, !srcloc !1
  ret void
}

; Function Attrs: nounwind uwtable
define void @_Z1fv(%struct.generator* noalias nocapture sret %agg.result) #2 personality i32 (...)* @__gxx_personality_v0 {
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
  %6 = getelementptr inbounds %struct.generator, %struct.generator* %agg.result, i64 0, i32 0, i32 0, i32 0
  store i8* %1, i8** %6, align 8, !alias.scope !2
  %7 = getelementptr i8, i8* %1, i64 16
  %8 = bitcast i8* %7 to i32*
  store i32 1, i32* %8, align 4
  ret void
}

; Function Attrs: nounwind
declare i8* @llvm.coro.elide() #3

; Function Attrs: nounwind
declare i8* @llvm.coro.init(i8*) #3

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

; Function Attrs: norecurse nounwind uwtable
define internal fastcc void @_Z1fv.resume(%_Z1fv.frame* nocapture nonnull %frame.ptr.resume) #4 personality i32 (...)* @__gxx_personality_v0 {
resume.entry:
  %0 = getelementptr %_Z1fv.frame, %_Z1fv.frame* %frame.ptr.resume, i64 0, i32 2
  %index = load i32, i32* %0, align 4
  %switch = icmp eq i32 %index, 1
  %i.0.spill.alloca252 = getelementptr %_Z1fv.frame, %_Z1fv.frame* %frame.ptr.resume, i64 0, i32 4
  br i1 %switch, label %for.cond.thread, label %for.cond

for.cond.thread:                                  ; preds = %resume.entry
  store i32 3, i32* %i.0.spill.alloca252, align 4
  br label %for.body

for.cond:                                         ; preds = %resume.entry
  %i.0.spill = load i32, i32* %i.0.spill.alloca252, align 4
  %mul = shl nsw i32 %i.0.spill, 1
  store i32 %mul, i32* %i.0.spill.alloca252, align 4
  %cmp = icmp slt i32 %mul, 200
  br i1 %cmp, label %for.body, label %exit

for.body:                                         ; preds = %for.cond.thread, %for.cond
  %i.04 = phi i32 [ 3, %for.cond.thread ], [ %mul, %for.cond ]
  %current_value.i = getelementptr %_Z1fv.frame, %_Z1fv.frame* %frame.ptr.resume, i64 0, i32 3, i32 0
  store i32 %i.04, i32* %current_value.i, align 4, !tbaa !5
  br label %exit

exit:                                             ; preds = %for.cond, %for.body
  %storemerge = phi i32 [ 2, %for.body ], [ 0, %for.cond ]
  store i32 %storemerge, i32* %0, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1fv.destroy(%_Z1fv.frame* nonnull %frame.ptr.destroy) #2 personality i32 (...)* @__gxx_personality_v0 {
destroy.entry:
  %0 = bitcast %_Z1fv.frame* %frame.ptr.destroy to i8*
  %call.i22 = tail call i32 (i8*, ...) @printf(i8* nonnull getelementptr inbounds ([11 x i8], [11 x i8]* @.str.1, i64 0, i64 0), %_Z1fv.frame* nonnull %frame.ptr.destroy) #3
  tail call void @free(i8* %0) #3
  ret void
}

; Function Attrs: nounwind uwtable
define internal fastcc void @_Z1fv.cleanup(%_Z1fv.frame* nonnull %frame.ptr.cleanup) #2 personality i32 (...)* @__gxx_personality_v0 {
destroy.entry:
  ret void
}

attributes #0 = { nobuiltin nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (https://github.com/GorNishanov/clang.git 7007ac36c6072caf31bb9bf95a3aa13361c7b5c7) (https://github.com/GorNishanov/llvm.git b9c40a2bc94c548377501092d0438371e9e93327)"}
!1 = !{i32 1822}
!2 = !{!3}
!3 = distinct !{!3, !4, !"_ZN9generatorIiE12promise_type17get_return_objectEv: %agg.result"}
!4 = distinct !{!4, !"_ZN9generatorIiE12promise_type17get_return_objectEv"}
!5 = !{!6, !7, i64 0}
!6 = !{!"_ZTSN9generatorIiE12promise_typeE", !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
