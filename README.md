A program in C++ that implements the scanning(Tokenization) step of a custom compiler built to compile the TINY programming language 

### Program input: 
The scanner will take a source code file written in the TINY programming language as an input; You can find an example of an input file in the file called `input.txt`


### Program output: 
The scanner should output a text file that contains the tokens found in the input file with each token represented on a single line in the following format
[number of line the token was found in starting from 1 in square brackets] [the actual string that represents the token in the input file] [a string that represents the token type in round brackets] 

### Example :

#### Program input:
~~~
 { Sample program
  in TINY language
  compute factorial
}

read x; {input an integer}
if 0<x then {compute only if x>=1}
  fact:=1;
  repeat
    fact := fact * x;
    x:=x-1
  until x=0;
  write fact {output factorial}
end
~~~
#### Program output: 
~~~
[1] { (LeftBrace)
[4] } (RightBrace)
[6] read (Read)
[6] x (ID)
[6] ; (SemiColon)
[6] { (LeftBrace)
[6] } (RightBrace)
[7] if (If)
[7] 0 (Num)
[7] < (LessThan)
[7] x (ID)
[7] then (Then)
[7] { (LeftBrace)
[7] } (RightBrace)
[8] fact (ID)
[8] := (Assign)
[8] 1 (Num)
[8] ; (SemiColon)
[9] repeat (Repeat)
[10] fact (ID)
[10] := (Assign)
[10] fact (ID)
[10] * (Times)
[10] x (ID)
[10] ; (SemiColon)
[11] x (ID)
[11] := (Assign)
[11] x (ID)
[11] - (Minus)
[11] 1 (Num)
[12] until (Until)
[12] x (ID)
[12] = (Equal)
[12] 0 (Num)
[12] ; (SemiColon)
[13] write (Write)
[13] fact (ID)
[13] { (LeftBrace)
[13] } (RightBrace)
[14] end (End)
~~~