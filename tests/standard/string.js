// string.estest

// 15.5
// test string.1
s=String()
typeof(s)=="string" && s==""

// test string.2
s=String("aha")
typeof(s)=="string" && s=="aha"

// test string.3
s=String(123.5)
typeof(s)=="string" && s=="123.5"

// test string.4
s= new String()
typeof(s)=="object" && s==""

// test string.5
s= new String("aha")
typeof(s)=="object" && s=="aha" && s.toString()=="aha";

// test string.6
s= new String(123.5)
typeof(s)=="object" && s=="123.5"

// test string.7
String.prototype.constructor==String

// test string.8
s = String.fromCharCode(80, 81)
s == "PQ"

// test string.9
s.valueOf()==s.toString();

// test string.10
("abc").length==3;


// test indexof.1
i = ("1234512345").indexOf("3")
i==2;

// test indexof.2
i = ("12345").indexOf("a")
i==-1;

// test indexof.3
i = ("123451122334455").indexOf("33")
i==9;

// test indexof.4
i = ("123451122334455").indexOf("3",5)
i==9;

// test indexof.5
f = String.prototype.indexOf;
Number.prototype.indexOf = f;
n = new Number(12345);
n.indexOf(3)==2

// test indexof.6
"abc".indexOf()==-1;

// test lastindexof.1
i = ("1234512345").lastIndexOf("3")
i==7;

// test lastindexof.2
i = ("12345").lastIndexOf("a")
i==-1;

// test lastindexof.3
i = ("1234512345").lastIndexOf("3", NaN)
i==7;

// test lastindexof.4
i = ("123451122334455").lastIndexOf("3",5)
i==2;

// test lastindexof.5
f = String.prototype.lastIndexOf;
Number.prototype.lastIndexOf = f;
n = new Number(12345);
n.lastIndexOf(3)==2

// test lastindexof.6
"abc".lastIndexOf()==-1;


// test split.1
s=new String("abcde");
a = s.split();
a[0]=="abcde";

// test split.2
s=new String("abcde");
a = s.split("");
a=="a,b,c,d,e";

// test split.3
s=new String("a,bc,,de");
a = s.split(",");
a.join("@")=="a@bc@@de";

// test split.4
s=new String("a@@b@c@@@@@de");
a = s.split("@@");
a.join("$")=="a$b@c$$@de";


// test substring.1
s = new String("abcdefgh");
s.substring()==s;

// test substring.2
s.substring(-3)==s;

// test substring.3
s.substring("albert")==s;

// test substring.4
s.substring(1234323232)=="";

// test substring.5
s.substring(3)=="defgh";

// test substring.6
s.substring(3,5)=="de";

// test substring.7
s.substring(5,3)=="de";

// test tolowercase.1
s = ("AbCd").toLowerCase();
s=="abcd" && typeof(s)=="string";

// test touppercase.1
s = ("AbCd").toUpperCase();
s=="ABCD" && typeof(s)=="string";

