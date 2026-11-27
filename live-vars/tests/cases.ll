define i32 @diamond(i1 %cond, i32 %a, i32 %b) {
entry:
  br i1 %cond, label %left, label %right
left:
  br label %merge
right:
  br label %merge
merge:
  %x = phi i32 [ %a, %left ], [ %b, %right ]
  ret i32 %x
}

define i32 @loop(i32 %n) {
entry:
  br label %loop
loop:
  %i = phi i32 [ 0, %entry ], [ %next, %loop ]
  %next = add i32 %i, 1
  %done = icmp eq i32 %next, %n
  br i1 %done, label %exit, label %loop
exit:
  ret i32 %i
}

define i32 @memory(i32 %x) {
entry:
  %slot = alloca i32
  store i32 %x, ptr %slot
  %value = load i32, ptr %slot
  ret i32 %value
}

define i32 @unnamed(i32 %0) {
  %2 = add i32 %0, 1
  ret i32 %2
}
