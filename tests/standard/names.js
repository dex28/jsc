// names.estest

// 13
// test function.1
function f1() {
  return true;
}

f1(false);

// test function.2
function f2(a, b, c, d) {
   return a || b || c || d;
}

f2(false,true,false,false);

// test function.3
f2.length == 4;

// 10.1.6
// test function.4
function f3(a, b, c) {
   arguments.a == arguments.c && arguments.a != arguments.b
}
f3 (2.4,5,2.4);

// test function.5
b = 0;
function f4(a) {
   if (a==0) {
      return true;
   } else {
      b++;
      return (a-1); //callee(a-1);
   }
}
f4(3) && (b == 3);


// test function.6
function f5() {
  arguments.length==3 && arguments[2]==5;
}
f5(1,"aa",5);


// 10.2
// test context.1
a = 1;
this.a==a;

// test context.2
function fc1(x) {
   this.a == x;
}
fc1(a);

// test context.3

vlocal = 1;
function fc2(v) {
  vglobal = v;
  var vlocal = v;
}
fc2(3);
vlocal == 1 && vglobal ==3;

// 15.3.2.1
// test new.1
a = 1;
function n(x) {
  this.a = x;
}
o = new n(3);
a==1 && o.a==3;

//10.1.3
// test hidden.1
var h1;
function h1() {
  return 16;
}
function h2() {
   return 17;
}
var h2;
h1()+1==h2();

// test hidden.2
var hp1 =1;
function h3 (hp1, hp2) {
   var hp2;
   return hp1+hp2;
}
h3(3,4)==7;

// test hidden.3
var hp1 =1;
function h3 (hp1, hp2, p3) {
   var hp1 = p3;
   return hp1+hp2;
}
h3(3,4,5)==9 && hp1==1;

// test object.1
a = new Object();
a.b = new Object();
a.b.c = 3;
a.b.c==3;

// test object.2
(a.b).c==3;






