@cell = global i32 0

define i32 @straight() {
entry:
  %slot = alloca i32
  store i32 7, ptr %slot
  %x = load i32, ptr %slot
  %sum = add i32 %x, 5
  %result = mul i32 %sum, 2
  ret i32 %result
}

define i32 @branch() {
entry:
  %condition = icmp eq i32 3, 3
  br i1 %condition, label %yes, label %no
yes:
  br label %merge
no:
  br label %merge
merge:
  %x = phi i32 [ 42, %yes ], [ 99, %no ]
  ret i32 %x
}

define i32 @unknown(i1 %condition, i32 %x) {
entry:
  br i1 %condition, label %yes, label %no
yes:
  br label %merge
no:
  br label %merge
merge:
  %value = phi i32 [ %x, %yes ], [ 8, %no ]
  ret i32 %value
}

; The definition is printed after its use, so propagation needs another scan.
define i32 @backwards_order() {
entry:
  br label %definition
use:
  %result = mul i32 %value, 3
  ret i32 %result
definition:
  %value = add i32 2, 4
  br label %use
}

define i32 @same_phi(i1 %condition) {
entry:
  br i1 %condition, label %yes, label %no
yes:
  br label %merge
no:
  br label %merge
merge:
  %value = phi i32 [ 5, %yes ], [ 5, %no ]
  %result = add i32 %value, 1
  ret i32 %result
}

; Each incoming constant is computed in a different predecessor. The merged
; result must flow to the successor even though neither input dominates it.
define i32 @edge_constants(i1 %condition) {
entry:
  br i1 %condition, label %left, label %right
left:
  %a = add i32 3, 4
  br label %join
right:
  %b = mul i32 7, 1
  br label %join
join:
  %v = phi i32 [ %a, %left ], [ %b, %right ]
  br label %exit
exit:
  %result = add i32 %v, 2
  ret i32 %result
}

define i32 @different_edges(i1 %condition) {
entry:
  br i1 %condition, label %left, label %right
left:
  %a = add i32 3, 4
  br label %join
right:
  %b = add i32 3, 5
  br label %join
join:
  %v = phi i32 [ %a, %left ], [ %b, %right ]
  ret i32 %v
}

define i32 @overflow() {
entry:
  %result = add nsw i32 2147483647, 1
  ret i32 %result
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
  ret i32 %next
}

define void @write_cell() {
entry:
  store i32 9, ptr @cell
  ret void
}

define i32 @memory() {
entry:
  store i32 1, ptr @cell
  call void @write_cell()
  %value = load i32, ptr @cell
  ret i32 %value
}

define void @write_pointer(ptr %p) {
entry:
  store i32 11, ptr %p
  ret void
}

define i32 @escaping_slot() {
entry:
  %slot = alloca i32
  store i32 1, ptr %slot
  call void @write_pointer(ptr %slot)
  %result = load i32, ptr %slot
  ret i32 %result
}

define i32 @main() {
entry:
  %a = call i32 @straight()
  %b = call i32 @branch()
  %c = call i32 @unknown(i1 true, i32 17)
  %d = call i32 @unknown(i1 false, i32 17)
  %e = call i32 @loop(i32 4)
  %f = call i32 @memory()
  %g = call i32 @backwards_order()
  %h = call i32 @same_phi(i1 false)
  %i = call i32 @escaping_slot()
  %j = call i32 @edge_constants(i1 true)
  %k = call i32 @edge_constants(i1 false)
  %l = call i32 @different_edges(i1 true)
  %m = call i32 @different_edges(i1 false)
  %a_ok = icmp eq i32 %a, 24
  %b_ok = icmp eq i32 %b, 42
  %c_ok = icmp eq i32 %c, 17
  %d_ok = icmp eq i32 %d, 8
  %e_ok = icmp eq i32 %e, 4
  %f_ok = icmp eq i32 %f, 9
  %g_ok = icmp eq i32 %g, 18
  %h_ok = icmp eq i32 %h, 6
  %i_ok = icmp eq i32 %i, 11
  %j_ok = icmp eq i32 %j, 9
  %k_ok = icmp eq i32 %k, 9
  %l_ok = icmp eq i32 %l, 7
  %m_ok = icmp eq i32 %m, 8
  %ab = and i1 %a_ok, %b_ok
  %cd = and i1 %c_ok, %d_ok
  %ef = and i1 %e_ok, %f_ok
  %abcd = and i1 %ab, %cd
  %gh = and i1 %g_ok, %h_ok
  %abcdef = and i1 %abcd, %ef
  %ghi = and i1 %gh, %i_ok
  %jk = and i1 %j_ok, %k_ok
  %lm = and i1 %l_ok, %m_ok
  %jklm = and i1 %jk, %lm
  %existing_ok = and i1 %abcdef, %ghi
  %ok = and i1 %existing_ok, %jklm
  %result = select i1 %ok, i32 0, i32 1
  ret i32 %result
}
