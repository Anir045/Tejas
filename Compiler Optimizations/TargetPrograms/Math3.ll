//LL IR, because Clang even with -O0 kept optimizing CF

define i32 @calculate() {
entry:
    %a = add i32 30, 12
    %b = sdiv i32 %a, 3
    %c = add i32 23, 77
    %d = sdiv i32 %c, 10
    %e = add i32 %b, %d
    %f = mul i32 %e, 30
    %g = sub i32 100, 95
    %h = add i32 %f, %g
    ret i32 %h
}