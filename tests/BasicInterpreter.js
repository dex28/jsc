// BASIC COMMANDS AND FUNCTIONS IMPLEMENTED: (* means completed)
//
// * ABS( number )
// * ACOS( number )
// * ASIN( number )
// * ATAN( number )
// * CLS
// * COS( number )
// * END
// * EXP( number )
// * FOR counter = start TO finish STEP increment
// * GOSUB line
// * GOTO line
//  IF expression THEN statement ELSE statement
// * LEFT( string$, number-of-spaces )
// * LEN( string$ )
// * LET variable = expression
// * LOG( number )
// * MID( string$, start-position-in-string, number-of-spaces )
// * NEXT counter
// * PRINT expressions...
// * REM string
// * RETURN
// * RIGHT( string$, number-of-spaces )
// * RND
// * ROUND( number )
// * SIN( number )
// * SQR( number )
// * STOP
// * TAN( number )
// * VAL( string$ )
//  WEND
//  WHILE expression

// ****************************************************
// **               Global variables                 **
// ****************************************************
CurrentForm=0;
CurrentLine=1;
Stack="9999";

// ****************************************************
// **                BASIC variables                 **
// ****************************************************
a=0;
b=0;
c=0;
d=0;
e=0;
f=0;
g=0;
h=0;
i=0;
j=0;
k=0;
l=0;
m=0;
n=0;
o=0;
p=0;
q=0;
r=0;
s=0;
t=0;
u=0;
v=0;
w=0;
x=0;
y=0;
z=0;

// ****************************************************
// **              Create BASIC object               **
// ****************************************************
function BASIC(){}

// BASIC functions and commands
function abs(parms){ return Math.abs(parms) }
function acos(parms){ return Math.acos(parms) }
function asin(parms){ return Math.asin(parms) }
function atan(parms){ return Math.atan(parms) }
function cos(parms){ return Math.cos(parms) }
function exp(parms){ return Math.exp(parms) }
function left(text,num){ return text.substring(0,eval(num)) }
function len(parms){ return parms.length }
function log(parms){ return Math.log(parms) }
function mid(text,start,num){ return text.substring(eval(start),eval(start)+eval(num)) }
function right(text,num){ return text.substring(text.length-eval(num),text.length) }
function rnd(parms){ return Math.random(parms) }
function round(parms){ return Math.round(parms) }
function sin(parms){ return Math.sin(parms) }
function sqr(parms){ return Math.sqrt(parms) }
function tan(parms){ return Math.tan(parms) }
function val(parms){ return eval(parms) }

function BASIC_CLS(){ CurrentForm.OutputBox.value="" }
BASIC.CLS=BASIC_CLS;

function BASIC_END(){ CurrentLine=9999 }
BASIC.END=BASIC_END;

function BASIC_FOR(parms){ BASIC_LET(GetWord(parms,1)+" "+GetWord(parms,2)) }
BASIC.FOR=BASIC_FOR;

function BASIC_GOSUB(LineNumber){
  Push(CurrentLine);
  BASIC_GOTO(LineNumber);
}
BASIC.GOSUB=BASIC_GOSUB;

function BASIC_GOTO(LineNumber){
  OldLine=CurrentLine;
  CurrentLine=1;
  while(CurrentLine<9999){
    if(GetWord(GetLine(CurrentLine),1)==LineNumber){
       CurrentLine--;
       return;
    }
    CurrentLine++;
  }
  CurrentLine=OldLine;
  alert("Error in "+GetLine(CurrentLine));
}
BASIC.GOTO=BASIC_GOTO;

function BASIC_LET(parms){
  var variable=GetWord(parms,1);
  if(variable=="a" || variable=="A") a=eval(GetWord(parms,2));
  if(variable=="b" || variable=="B") b=eval(GetWord(parms,2));
  if(variable=="c" || variable=="C") c=eval(GetWord(parms,2));
  if(variable=="d" || variable=="D") d=eval(GetWord(parms,2));
  if(variable=="e" || variable=="E") e=eval(GetWord(parms,2));
  if(variable=="f" || variable=="F") f=eval(GetWord(parms,2));
  if(variable=="g" || variable=="G") g=eval(GetWord(parms,2));
  if(variable=="h" || variable=="H") h=eval(GetWord(parms,2));
  if(variable=="i" || variable=="I") i=eval(GetWord(parms,2));
  if(variable=="j" || variable=="J") j=eval(GetWord(parms,2));
  if(variable=="k" || variable=="K") k=eval(GetWord(parms,2));
  if(variable=="l" || variable=="L") l=eval(GetWord(parms,2));
  if(variable=="m" || variable=="M") m=eval(GetWord(parms,2));
  if(variable=="n" || variable=="N") n=eval(GetWord(parms,2));
  if(variable=="o" || variable=="O") o=eval(GetWord(parms,2));
  if(variable=="p" || variable=="P") p=eval(GetWord(parms,2));
  if(variable=="q" || variable=="Q") q=eval(GetWord(parms,2));
  if(variable=="r" || variable=="R") r=eval(GetWord(parms,2));
  if(variable=="s" || variable=="S") s=eval(GetWord(parms,2));
  if(variable=="t" || variable=="T") t=eval(GetWord(parms,2));
  if(variable=="u" || variable=="U") u=eval(GetWord(parms,2));
  if(variable=="v" || variable=="V") v=eval(GetWord(parms,2));
  if(variable=="w" || variable=="W") w=eval(GetWord(parms,2));
  if(variable=="x" || variable=="X") x=eval(GetWord(parms,2));
  if(variable=="y" || variable=="Y") y=eval(GetWord(parms,2));
  if(variable=="z" || variable=="Z") z=eval(GetWord(parms,2));
}
BASIC.LET=BASIC_LET;

function BASIC_NEXT(parms){
  var OldLine=CurrentLine;
  for(CurrentLine=OldLine-1;CurrentLine>0;CurrentLine--){
    LineText=GetLine(CurrentLine);
    if(GetWord(LineText.toLowerCase(),2)=="for"){
      if(GetWord(LineText.toLowerCase(),3)==parms.toLowerCase()){
        inc=eval(parms)+eval(GetWord(LineText,8));
        BASIC_LET(parms+" "+inc);
        if(inc>eval(GetWord(LineText,6))) CurrentLine=OldLine;
        return;                                  65
      }
    }
  }
  alert("Unable to locate start of FOR loop!");
  CurrentLine=9999;
}
BASIC.NEXT=BASIC_NEXT;

function BASIC_PRINT(text){
  if(text.charAt(0)!="\"") text=text.toLowerCase();  
  if(text.charAt(text.length-1)==";"){
    CurrentForm.OutputBox.value+=eval(text.substring(0,text.length-1));
  }else {
    CurrentForm.OutputBox.value+=eval(text)+unescape("%0D")+unescape("%0A")
  }
}
BASIC.PRINT=BASIC_PRINT;

function BASIC_REM(){}
BASIC.REM=BASIC_REM;

function BASIC_RETURN(){
  CurrentLine=Pop();
}
BASIC.RETURN=BASIC_RETURN;

function BASIC_STOP(){ CurrentForm.Trace.value="Trace On" }
BASIC.STOP=BASIC_STOP;

// ****************************************************
// **              Non-BASIC functions               **
// ****************************************************
function Push(parms){
  Stack=parms+" "+Stack;
}

function Pop(){
  parms=GetWord(Stack,1);
  Stack=Stack.substring(parms.length+1,Stack.length);
  return parms;
}

function GetLine(LineNumber){
  var count=0;
  var start=0;
  for(cntr=0;cntr<CurrentForm.ProgramLines.value.length;cntr++){
    if(CurrentForm.ProgramLines.value.charAt(cntr)==unescape("%0D")){
      count++;
      if(start==0) start=-1;
      if(count==LineNumber-1){
        start=cntr+2;
      }else if(count==LineNumber){
        return CurrentForm.ProgramLines.value.substring(start,cntr);
      }
    }
  }
  if(start>0){
    return CurrentForm.ProgramLines.value.substring(start,CurrentForm.ProgramLines.value.length);
  }else if(start==0 && LineNumber==1) {
    return CurrentForm.ProgramLines.value;    
  }else {
    return "9999 END";
  }
}

function GetWord(text,WordNum){
  var count=0;
  var start=0;
  var flag=0;
  while(text.charAt(0)==" ") text=text.substring(2,text.length);
  for(cntr=0;cntr<text.length;cntr++){
    if(text.charAt(cntr)==" " || text.charAt(cntr)=="="){
      if(flag==0){
        count++;
        while(text.charAt(cntr+1)==" " || text.charAt(cntr+1)=="=") cntr++;
        if(text.charAt(cntr)=="\""){
          flag=1;
          cntr++;
        }
        if(count==WordNum-1){
          start=cntr+1;
        }else if(count==WordNum){
          return text.substring(start,cntr);
        }
      }
    }else if(text.charAt(cntr)=="\""){
      if(flag==1){
        flag=0;
      }else {
        flag=1;
      }
    }
  }
  if(start!=0){
    return text.substring(start,text.length);
  }else {
    return "";
  }
}

function Execute(LineText){
  var parm="";
  var parms="";
  var cntr=3;
  if(CurrentForm.Trace.value=="Trace On") CurrentForm.InputBox.value=LineText;
  command=GetWord(LineText.toUpperCase(),2);
  if(command.length<2) return;
  parm=GetWord(LineText,3);
  parms=parm;
  while(parm!=""){
    cntr++;
    parm=GetWord(LineText,cntr);
    if(parm!="") parms=parms+" "+parm;
  }
  BASIC[command](parms);
}

function RunProgram(form){
  CurrentForm=form;
  if(CurrentForm.Trace.value=="Trace Off"){
    CurrentLine=1
    while(CurrentLine<9999){
      Execute(GetLine(CurrentLine));
      CurrentLine++;
      if(CurrentForm.Trace.value=="Trace On") return;
    }
  }else {
    Execute(GetLine(CurrentLine));
    CurrentLine++;
  }
  if(CurrentLine>=9999){
    alert("Program Terminated Normally!");
    CurrentLine=1;
  }
}

function ToggleTrace(form){
  if(form.Trace.value=="Trace Off"){
    form.Trace.value="Trace On";
  }else {
    form.Trace.value="Trace Off";
  }
}