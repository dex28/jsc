// array.estest

// 15.4
// test array.1
a = Array(1,2,3)
String(a)=="1,2,3" && a.length == 3 && a[1]==2;

// test array.2
a = new Array(1,2,3,4)
a.toString()=="1,2,3,4" && a.length==4 && a[1]==2;

// test array.3
a = new Array(12)
a.length==12;

// test array.4
a = new Array()
a.length==0 && a.join()=="";

// test array.5
a = new Array("aa");
a.length==1 && a[0]=="aa";

// test array.6
//Array.length == 1 && Array.prototype.length==0;

// test array.7
a = new Array(1,"aa",12.5);
a.join("// // ")=="1// // aa// // 12.5"

// test array.8
//a.reverse().toString()=="12.5,aa,1";

// test array.9
//a.sort().join()=="1,12.5,aa";

// test array.10
a[10]="dix";
a.length==11;

// test array.11
a.length=20;
a[10]=="dix" && a.length==20;

// test array.12
a.toString()=="1,12.5,aa,,,,,,,,dix,,,,,,,,,";

// test array.13
a.length=4;
a.join("$")=="1$12.5$aa$";

