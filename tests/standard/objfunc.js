// objfunc.estest

// 15.2
// test object.1
o1 = Object();
o2 = Object(null);
o1!=o2 && o1.constructor == Object && typeof(o2)=="object";

// test object.2
s1 = new String("aha");
o3 = new Object(s1);
o3==s1 && typeof(s1)=="object";

// test object.3
s2 = "aString";
o4 = new Object(s2);
typeof(o4)=="object" && o4=="aString";

// test object.4
n1 = 12.5;
o5 = new Object(n1);
typeof(o5)=="object" && o5==12.5;

// test object.5
b1 = true;
o6 = new Object(b1);
(typeof(o6)=="object") && o6;

// test object.6
Object.prototype.constructor == Object;

// test object.7
o7 = new Object();
o7.a="aa";
o8 = o7.valueOf();
o7==o8;

// test object.8
function oc () {
    this.a = "A";
    this.b = "B";
    this.c = new String();
}
oo = new oc();
oo.a == "A";


// 15.2.5.
// test function.1
f1 = Function("x","x+1");
f1(2)==3 && typeof(f1)=="function"

// test function.2
f2 = new Function("x","x+1");
f2(2)==3;

// test function.3
f3 = new Function("x","y", "x+y+1");
f3(3,4)==8;

// test function.4
f4 = new Function("x,y", "x+y+1");
f4(3,4)==8;

// test function.5
f5 = new Function("x ","  y ", "x+y+1");
f5(3,4)==8;

// test function.6
f6 = new Function(" x, y ", "x+y+1");
f6(3,4)==8 && f6.length==2;

// test function.7
f7 = new Function("a", "this.p = a+1");
of7 = new f7(2);
of7.p==3 && typeof(of7)=="object";

// test function.8
f7p = new Object();
f7p.f = new Function("this.p + 5");
f7.prototype = f7p;
f7o = new f7(2);
f7o.f()==8

// test function.9
Function.length==1

// test function.10
typeof(Function.prototype(1,2,3))=="undefined";

// test function.11
function f11(x,y) {doIt(x,y);}
f11.toString().indexOf("doIt")>8;

// test function.12
f11.toString().indexOf("f11")>8;

// test function.13
f13 = new Function("x","y","doIt(x,y)")
f13.toString().indexOf("anonymous")>6;

// test function.14
// Just test proper parsing of empty functions
function e(){}
true;

// test arguments.1
String(f7.arguments)=="null";

// test arguments.2
function fa1(x,y) {gfa1=arguments;};
fa1(2,3);
gfa1.length==2 && gfa1[0]==2 && gfa1[1]==3;

// test arguments.3
with (gfa1) {
   length==2;
}

// test prototype.1
String.prototype.tag = new Function('x',"'<'+x+'>'+this+'</'+x+'>'");
s1 = new String("hello");
s1.tag('B')=='<B>hello</B>';

// test prototype.2
("title").tag("H1")=='<H1>title</H1>';

// test new.1
function n1() {
   this.a = 1;
   return this;
}
nn = new n1();
nn.a==1;

