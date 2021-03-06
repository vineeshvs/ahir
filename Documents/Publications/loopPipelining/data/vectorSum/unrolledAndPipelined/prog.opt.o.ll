; ModuleID = 'prog.opt.o'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i386-pc-linux-gnu"

@.str = private constant [13 x i8] c"in_data_pipe\00"
@A = common global [64 x float] zeroinitializer, align 4
@B = common global [64 x float] zeroinitializer, align 4
@C = common global [64 x float] zeroinitializer, align 4
@.str1 = private constant [14 x i8] c"out_data_pipe\00"

define void @getData() nounwind {
bb.nph:
  br label %0

; <label>:0                                       ; preds = %0, %bb.nph
  %idx.01 = phi i32 [ 0, %bb.nph ], [ %2, %0 ]
  %scevgep2 = getelementptr [64 x float]* @B, i32 0, i32 %idx.01
  %scevgep = getelementptr [64 x float]* @A, i32 0, i32 %idx.01
  %1 = tail call float @read_float32(i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0)) nounwind
  store float %1, float* %scevgep, align 4
  store float %1, float* %scevgep2, align 4
  %2 = add nsw i32 %idx.01, 1
  %exitcond1 = icmp eq i32 %2, 64
  br i1 %exitcond1, label %._crit_edge, label %0

._crit_edge:                                      ; preds = %0
  ret void
}

declare float @read_float32(i8*)

define void @sendResult() nounwind {
bb.nph:
  br label %0

; <label>:0                                       ; preds = %0, %bb.nph
  %idx.01 = phi i32 [ 0, %bb.nph ], [ %2, %0 ]
  %scevgep = getelementptr [64 x float]* @C, i32 0, i32 %idx.01
  %1 = load float* %scevgep, align 4
  tail call void @write_float32(i8* getelementptr inbounds ([14 x i8]* @.str1, i32 0, i32 0), float %1) nounwind
  %2 = add nsw i32 %idx.01, 1
  %exitcond1 = icmp eq i32 %2, 64
  br i1 %exitcond1, label %._crit_edge, label %0

._crit_edge:                                      ; preds = %0
  ret void
}

declare void @write_float32(i8*, float)

define void @vectorSum() noreturn nounwind {
; <label>:0
  br label %sendResult.exit

sendResult.exit.loopexit:                         ; preds = %3
  br label %sendResult.exit.backedge

sendResult.exit:                                  ; preds = %sendResult.exit.backedge, %0
  %idx.01.i = phi i32 [ 0, %0 ], [ %idx.01.i.be, %sendResult.exit.backedge ]
  %scevgep.i = getelementptr [64 x float]* @A, i32 0, i32 %idx.01.i
  %scevgep2.i = getelementptr [64 x float]* @B, i32 0, i32 %idx.01.i
  %1 = tail call float @read_float32(i8* getelementptr inbounds ([13 x i8]* @.str, i32 0, i32 0)) nounwind
  store float %1, float* %scevgep.i, align 4
  store float %1, float* %scevgep2.i, align 4
  %2 = add nsw i32 %idx.01.i, 1
  %exitcond.i = icmp eq i32 %2, 64
  br i1 %exitcond.i, label %getData.exit, label %sendResult.exit.backedge

sendResult.exit.backedge:                         ; preds = %sendResult.exit, %sendResult.exit.loopexit
  %idx.01.i.be = phi i32 [ %2, %sendResult.exit ], [ 0, %sendResult.exit.loopexit ]
  br label %sendResult.exit

getData.exit:                                     ; preds = %sendResult.exit
  tail call void @_vectorSum_()
  br label %3

; <label>:3                                       ; preds = %3, %getData.exit
  %idx.01.i1 = phi i32 [ 0, %getData.exit ], [ %5, %3 ]
  %scevgep.i2 = getelementptr [64 x float]* @C, i32 0, i32 %idx.01.i1
  %4 = load float* %scevgep.i2, align 4
  tail call void @write_float32(i8* getelementptr inbounds ([14 x i8]* @.str1, i32 0, i32 0), float %4) nounwind
  %5 = add nsw i32 %idx.01.i1, 1
  %exitcond1 = icmp eq i32 %5, 64
  br i1 %exitcond1, label %sendResult.exit.loopexit, label %3
}

define void @_vectorSum_() nounwind {
bb.nph:
  br label %0

; <label>:0                                       ; preds = %0, %bb.nph
  %indvar = phi i32 [ 0, %bb.nph ], [ %indvar.next, %0 ]
  %tmp2 = mul i32 %indvar, 8
  %tmp3 = add i32 %tmp2, 7
  %scevgep31 = getelementptr [64 x float]* @C, i32 0, i32 %tmp3
  %scevgep30 = getelementptr [64 x float]* @B, i32 0, i32 %tmp3
  %scevgep29 = getelementptr [64 x float]* @A, i32 0, i32 %tmp3
  %tmp13 = add i32 %tmp2, 6
  %scevgep27 = getelementptr [64 x float]* @C, i32 0, i32 %tmp13
  %scevgep26 = getelementptr [64 x float]* @B, i32 0, i32 %tmp13
  %scevgep25 = getelementptr [64 x float]* @A, i32 0, i32 %tmp13
  %tmp25 = add i32 %tmp2, 5
  %scevgep23 = getelementptr [64 x float]* @C, i32 0, i32 %tmp25
  %scevgep22 = getelementptr [64 x float]* @B, i32 0, i32 %tmp25
  %scevgep21 = getelementptr [64 x float]* @A, i32 0, i32 %tmp25
  %tmp34 = add i32 %tmp2, 4
  %scevgep19 = getelementptr [64 x float]* @C, i32 0, i32 %tmp34
  %scevgep18 = getelementptr [64 x float]* @B, i32 0, i32 %tmp34
  %scevgep17 = getelementptr [64 x float]* @A, i32 0, i32 %tmp34
  %tmp38 = add i32 %tmp2, 3
  %scevgep15 = getelementptr [64 x float]* @C, i32 0, i32 %tmp38
  %scevgep14 = getelementptr [64 x float]* @B, i32 0, i32 %tmp38
  %scevgep13 = getelementptr [64 x float]* @A, i32 0, i32 %tmp38
  %tmp42 = add i32 %tmp2, 2
  %scevgep11 = getelementptr [64 x float]* @C, i32 0, i32 %tmp42
  %scevgep10 = getelementptr [64 x float]* @B, i32 0, i32 %tmp42
  %scevgep9 = getelementptr [64 x float]* @A, i32 0, i32 %tmp42
  %tmp46 = add i32 %tmp2, 1
  %scevgep7 = getelementptr [64 x float]* @C, i32 0, i32 %tmp46
  %scevgep6 = getelementptr [64 x float]* @B, i32 0, i32 %tmp46
  %scevgep5 = getelementptr [64 x float]* @A, i32 0, i32 %tmp46
  %scevgep3 = getelementptr [64 x float]* @C, i32 0, i32 %tmp2
  %scevgep2 = getelementptr [64 x float]* @B, i32 0, i32 %tmp2
  %scevgep = getelementptr [64 x float]* @A, i32 0, i32 %tmp2
  tail call void @loop_pipelining_on(i32 8) nounwind
  %1 = load float* %scevgep, align 4
  %2 = load float* %scevgep2, align 4
  %3 = fadd float %1, %2
  %4 = load float* %scevgep5, align 4
  %5 = load float* %scevgep6, align 4
  %6 = fadd float %4, %5
  %7 = load float* %scevgep9, align 4
  %8 = load float* %scevgep10, align 4
  %9 = fadd float %7, %8
  %10 = load float* %scevgep13, align 4
  %11 = load float* %scevgep14, align 4
  %12 = fadd float %10, %11
  %13 = load float* %scevgep17, align 4
  %14 = load float* %scevgep18, align 4
  %15 = fadd float %13, %14
  %16 = load float* %scevgep21, align 4
  %17 = load float* %scevgep22, align 4
  %18 = fadd float %16, %17
  %19 = load float* %scevgep25, align 4
  %20 = load float* %scevgep26, align 4
  %21 = fadd float %19, %20
  %22 = load float* %scevgep29, align 4
  %23 = load float* %scevgep30, align 4
  %24 = fadd float %22, %23
  store float %3, float* %scevgep3, align 4
  store float %6, float* %scevgep7, align 4
  store float %9, float* %scevgep11, align 4
  store float %12, float* %scevgep15, align 4
  store float %15, float* %scevgep19, align 4
  store float %18, float* %scevgep23, align 4
  store float %21, float* %scevgep27, align 4
  store float %24, float* %scevgep31, align 4
  %indvar.next = add i32 %indvar, 1
  %exitcond1 = icmp eq i32 %indvar.next, 8
  br i1 %exitcond1, label %._crit_edge, label %0

._crit_edge:                                      ; preds = %0
  ret void
}

declare void @loop_pipelining_on(i32)
