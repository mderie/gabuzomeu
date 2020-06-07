
----------------------------------------------------
Yet Another Esoteric Programming Language (YEAPL !-)
----------------------------------------------------

Source of inspirations
**********************

- Le cerveau Shadok a quatre cases !
https://www.youtube.com/watch?v=DuJDPqb0nvE

- Homage a Jacques Rouxel
https://fr.wikipedia.org/wiki/Les_Shadoks

- Litteral number converter :)
https://www.dcode.fr/shadoks-numeral-system

- Turing machine
https://rosettacode.org/wiki/Universal_Turing_machine

- The Pascal programming language (for the "#" usage)

- Brainfuck (a real gem :)

- x86 assembly language (for the capital letter habit)

- Dos batch (for the label usage)

4 unsigned 8 bit "registers" / Constants
****************************************

Finally not a good idea to have 4 numerical constants, so no need for brackets, ie [GA]

GA = 0
BU = 1
ZO = 2
MEU = 3
Idea : introduce NEXT as the next target cell ?

Flags

Zero
Carry ?

12 keywords (Final ?-)
**********************

SCAN (or PEAK) ==> SCAN appartment // Like READ but without moving the input pointer (may set ZF if any) [Optional]
SEEK ==> SEEK expression or maybe SEEK appartment ? Or finally SEEK literal // Move the read pointer (unsigned value, 0 = #GA is the beginning) [Optional]

1) JUMP (or GOTO) ==> JUMP label // Unconditional JUMP to the label, based on the ZF if set or not set ?

DICE (or RAND) ==> DICE appartment // Put a random value from 0 to 255 into the MEU register [Optional]
2) CALC (or EVAL) ==> appartment, expression // The only instruction that takes an math runtime expression !!! (classical operator & appartment combinations)

3) BIRD ==> BIRD appartment // Create a new bird and link the current one from MEU to MEU (may raise exception)... Or from GA to GA ??? Save one operand :)
4) FREE (or KILL) ==> FREE appartment // Destroy recursively the bird up or down linked to the given appartment

5) PUMP ==> PUMP appartment // Replace peak ? or read ?-)
6) DUMP (or BUMP) ==> DUMP appartment (add support for DUMP constant ?) // Replace write !-)

7) To be combined into one MOVE keyword since two arrow parts can't share the same apparment !!!
MORE ==> MORE appartment // Travel from the current bird to the next one using MEU as source (may raise exception)
LESS ==> LESS appartment // Travel from the current bird to the previous one using GA as target (may raise exception)

8) HEAD ==> HEAD appartment, label // Test if the BU appartement holds an arrow head (followed by ZERO or ELSE) and JUMP to label if true else continue the flow
9) TAIL ==> TAIL appartment, label // Test if the ZO appartement holds an arrow tail
BODY ==> BODY appartment, label // Test if the GA appartment holds neither an arrow head nor an arrow tail [Optional] Not mandatory for v1 since it can be replaced by HEAD & TAIL

10) ZERO ==> ZERO appartment, label // Test if MEU content is equal to zero and JUMP to label if true else continue the flow
11) ELSE ==> ELSE appartment, label // Test if GA content is different from zero and JUMP to label if true else continue the flow

12) LAST ==> LAST label // True/Set Zero Flag if input pointer is beyond the last character (TEST), JUMP to label if true else continue the flow
Idea : introduce another keyword that return the character count not yet processed ?

First ideas
***********

CALL label? ==> Unconditional JUMP to label and return ???
READ ==> READ(GA) or GA READ or READ [GA] (may set ZF)
REPL ? // Replace in place for input / output changing ?
COMP ? ==> COMP [BU], "A" or COMP [BU], 65 ? Or both ?
MOVE ==> MOVE [GA], GA
POKE ==> // Like WRIT? but without moving the output pointer, needed ?
WRITE (Find a four letter word here ? https://wordfinder.yourdictionary.com/letter-words/4/)
TURN (aka rotate) ==> TURN GA ? // Needed ?
UP (Find a four letter word here ? https://wordfinder.yourdictionary.com/letter-words/4/)
COUNT ? // Returns the total number of brains/birds
PUSH ==> PUSH MEU // Create a new brain/bird from current MEU (and jump to it ?)
PLOP (FLOP ?) ==> PLOP MEU // Destroy the brain/bird link associated to the MEU register (recursively)
INCR ==> INCR [GA] // Set the Zero flag if overflow ;)
DECR ==> DECR [MEU] // Set the Zero flag
TEST ==> TEST [BU] ? // Is the BU register holding a brain/bird

Futures
*******

REPEAT !
FEEL
COOL
LEAF
DUMB
BUMP
NEXT
PREV
FALL
BALL
MALL
NULL
BOIL
CELL (care largely used in source code)
LIST
FEED
FOOD
FREE
FULL
UNIQ
IFEQ
EQUAL
IFTH
TIME
DATE
DOWN
LINK
SPOT
BACK
FORE
EVAL
BUMP
LOAD
SAVE
BLOB
ZERO
POP
SCAN
PULL
LEFT
GOTO
LOOP
FLAG
DIFF
GRAB
STOP
CONT
FIST
COPY
CALL ==> CALL label
RETR // No param :(
COND
BIND
FIND
KIND
MIND
WIND
DROP
MESG
TALK
PORT
DUPL
ROLL
DICE
FACE
MAZE
BILL
PILL
FILL
BULL
FUNC
BUZZ
EXIT
FOOL :)
RULE
MULE
FROM
POOR
POOP
LOVE
PAIN
BUFF Idea bidi tmp storage ?
SKIP
CIAO (it means both hello & goodbye)
FILE
CODE
TEXT
DONE
TONE
BYTE Idea : Flip io to byte mode
CHAR Idea : Reverse of above
STAR
TEAR

Gross !
*******

SHIT
FUCK
CUNT
DICK
CRAP
BUTT
PISS
TITS
COCK
DAMN
SLUT
....

Enhancement ?
*************

Add an extra ? after keyword that can be tested...

Comments
********

C/C++ like : // at the end of line or /* & */ for line block
Or just ";" ? ==> Yes !

Charset (finally it is a platform concern)
******************************************

ISO-8859-1 (https://en.wikipedia.org/wiki/ISO/IEC_8859-1)

label (used for JUMP)
*********************

Preceded by a colon ==> ":loop" for the declaration and without colon for the definition (usage)

Exceptions
**********

Illegal "stack" operations
Illegal literal
Missing label
Seek out of bound
...

Termination
***********

When no more instruction !-) No need for exit 0 or stop for now...
A valid program may (begin and) end with a ":end" label for example.

IO
**

Virtual unlimited input and output characters (command line capacity is OS dependant)

Interpreter
***********

Command line based program that takes two arguments : a text file (*.gbzm) and an input string
between double quotes. The runtime loads the text file and provides the input string to the running gabuzomeu program.
Finally the "collected" string output is shown at the end

literals (numbers in the language)
**********************************

Characters : Enclosed in single quote ==> Finally not supported (voluntary obfuscation)
Numbers : Using the base four notation (GA BU ZO MEU related), preceded with ² or surrounded by """ but then the command line failed :(
==> for gabuzomeu experts only ;-)

0 = "GA"
1 = "BU"
2 = "ZO"
3 = "MEU"
4 = "BUGA"
5 = "BUBU"
27 = "GABUZOMEU" !-)
100 = "BUZOBUGA"
104 = "BUZOZOGA" = 'h'

or with one (maybe two #)

0 = #GA
1 = #BU
2 = #ZO
3 = #MEU
4 = #BUGA
5 = #BUBU
10 = #ZOZO = LF
13 = #MEUBU = CR
27 = #GABUZOMEU
31 = #BUMEUMEU = Bing !
100 = #BUZOBUGA = 'd'
104 = #BUZOZOGA = 'h'

or only gabuzomeu litteral between single quotes (to ease the command line ?)

Numbers from / to the outside
*****************************

The supported bases are 2, 4, 10, 16, 64 & 256 Where the later actually means make use of the ascii code for representing them.
Obviously numbers below ascii code 32 are not printable. They are shown as using gabuzomeu base 4 notation between #
For the others bases, since we don't know in advance their length, they must be enclosed as well by #
In order to please the padawan programmers, the default base is the fourth one for the code and 256 for the I/O
The BASE keyword allow a base change for further I/O operations Big numbers (arbitray long positive integers)
support can be set by using the -b or --big parameter else the default number size is the byte

Allowed bird links
******************

TODO: in ASCII art ?

Disallowed bird links
*********************

Multiple arrows head or tail within the same cell !
TODO: in ASCII art ?

Miscelleanous
*************

- One brain/bird at time, four appartements or braincells or simply cells (numbered from 0 to 3 ?) or just named ?-)
- Character collection processor ? Well do the word processor for the V2
- Each appartement could be either a single character storage or an appartment arrow
- Input pointer / output pointer (both with or without overwritting ?)
- After a TEST or COMP the JUMP instruction is a conditional one (same for LOOP ?)
- One instruction per line in a file ==> No ! The grammar is strong :)
- Or take the first input string as program with ";" as separator and the second command line parameter is the data ==> Do we really need instruction separator ?
==> byte stream in gabuzomeu notation (nibble based) ? #MEUBU#ZOZO ==> This allows to pass non printable characters :)
Again Pascal syntax is as follow 'Hello,'#13#10'world!' So we can mix double quotes and #values if they are in one block :)
- Case insensitive
- Accept ";" as instruction separator. Allow single line program !-)
- byte and char are exchangeable (but char is for the ouside world and (big) number is the only rule)
- When the progam starts there is only one bird and all of its appartments hold zero (as any new bird)
- input string can be empty !-)

Evolution
*********

Introduce a counter register ?
Introduce LOOP keyword ?

Care : command line limitation (OS related topic)
*************************************************

https://www.cyberciti.biz/faq/linux-unix-arg_max-maximum-length-of-arguments/
Try the following command 
getconf ARG_MAX

Under all the Windows...
https://stackoverflow.com/questions/3205027/maximum-length-of-command-line-string

In C/C++ the program entry point is as follow :
int main(int argc; char *argv[])

A Gabuzomeu program does not have input parameters nor return values
but it can read input value(s) and write output value(s). Those values
may be provided from / to file or from / to the console. See the GBZM "special" instruction [02/06/2020]

Greetings
*********

- Jarro2783 for the cxxopts library (https://github.com/jarro2783/cxxopts)
- R35382 for the bnflite library (https://github.com/r35382/bnflite)
- Sercantutar for the infint library (https://github.com/sercantutar/infint)
- ...
All demosceners, ...

That's all folks ! [Project started the ten of May 2020]