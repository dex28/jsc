// regexp.estest

// // test regexp
r = new RegExp("a(.?)b");
// r.toString() == "/abc/";

// test test.1
r.test("xabc");

// test test.2
!r.test("XABC");

// test test.3
//r.ignoreCase=true;
r.test("XABC");

// test ingoreCase
r.ignoreCase

// test search.1
//("XADBC").search(r)==1;

// test search.2
r.ignoreCase=false;
("XADBC").search(r)==-1;

// test match.1
a=("bbaabb").match(r)
a=="aab,a"

// test index.1
a.index==2;

// test input.1
a.input.toString()=="bbaabb"

// test exec.1
a=r.exec("bbaabb")
a=="aab,a"


