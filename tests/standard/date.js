// date.estest

// 15.9
// test date.1
Date().substring(0,10)==Date(1,2,4,5,3,2).substring(0,10);

// test date.2
//Date().substring(0,10)==(new Date()).toString().substring(0,10)

// test date.3

//d3 = new Date(1998,0,3,12,30,5,10);
d3 = new Date(1998,0,3,12,30,5);
/*d3.getFullYear()==1998 && */d3.getYear()==98 &&
d3.getMonth()==0 && d3.getDate()==3 && d3.getDay()==6 &&
d3.getHours()==12 && d3.getMinutes()==30 &&
d3.getSeconds()==5;// && d3.getMilliseconds()==10;

// test date.4
d3.valueOf()==d3.getTime();

// test date.5
Date.prototype.constructor == Date;

// Very simple tests to ensure that some string was generated
// test date.6
d3.toString().indexOf("98")>0;

// test date.7
d3.toLocaleString().indexOf("98")>0;

// test date.8
d3.toUTCString().indexOf("98")>0;

// test date.9
d3.toGMTString().indexOf("98")>0;

// test date.10
Date.UTC(1998,0,3,12,30,5,10) - d3.valueOf() == 3600000;

// test date.10
d4 = new Date(1,0,1);
d4.getYear() == 1;

// test date.10
d4 = new Date(100,0,1);
d4.getYear() == -1800 && d4.getFullYear() == 100;

// test date.11
d4 = new Date(1000,0,1);
d4.getYear() == -900 && d4.getFullYear() == 1000;

// test date.12
d4 = new Date(98,0,1);
d4.getYear() == 98  && d4.getFullYear() == 1998;

// test date.13
d4 = new Date(2000,0,1);
d4.getYear() == 100 && d4.getFullYear() == 2000;

// test date.14
d4 = new Date(98,0,1);
d4.setFullYear(98)
d4.getFullYear() == 98;
