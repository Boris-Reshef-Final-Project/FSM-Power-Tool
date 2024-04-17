
# FSM Parser & Power Reduction
## Author:   Reshef Schachter
## Created:  17/04/24
## Modified: 17/04/24
---
### General concept:

The Optimizer can work in 1 of 2 methods:
1. Split the states BEFORE parsing by creating 2 (or more) new KIS2 files.
2. Split the states WHILE  parsing.

Option 1 is preferable because it's more scalable, in this case the Parser is only resposible for converting a KIS2 file to VHDL.\
This means we can use it later for more than 2 CFSM, and it simplifies the parser which already has a lot of complicated work to\
do, such as: detect radix, state-naming, and other parameters.\
No need to make the parser's work harder by including the optimization.

![image](https://github.com/Boris-Reshef-Final-Project/FSM-Power-Tool/assets/73393286/c60dbf86-e63a-46e5-a9c1-adbf098ff36d)


---
### Progression steps

main source file = kis2vhd.cpp\
parser sub-function = fsm2process\
parser main function = kisFiles2vhd\
optimizer main function = optimizeFSM

1. Basic fsm2process that can create the content of the vhdl process based on a kiss file. (only simple state names)
2. Improve fsm2process by handling complex state names.
3. Improve fsm2process by adding the ability to handle "don't-care" logic in INPUTS.
4. Improve fsm2process by adding the ability to handle "don't-care" logic in UTPUTS.
5. Create Parser function kisFiles2vhd that creates the entire VHD file with ibraries, entity, architecture, declerations,
that can create multiple processes by using fsm2process for the content of each process. 
6. Create the function optimizeFSM that will create the necessary amount of ISS files (depending on user input).
The function takes as an input the original FSM and decomposes the FSM into the created destination KISS files
based on the decomposition algorithm that was chosen. The first algorithm to build is "cut in half" in which
the states are simply divided equally into 2 CFSM.
7. Create a universal test-bench to check FUNCTIONAL (and timing) equivalence y comparing to the un-optimized VHD file.
8. Add a new (and better) decomposition algorithm, and give the user the hoice of choosing between the different algorithms.
9. Repeat step 8 until we're satisfied with the results.
10. Make the code pretty and organized. Add comments to the code where ecessary.


Optional improvements:

1. Add GUI.
2. Let the user choose output state coding. (one-hot, one-cold, m-of-n, binary, etc...)
3. .......


---
### Pseudo code for 'main'

>the functions in this pseudo-code are written as pure function, but this is just for the sake of this explanation.\
>in reality these function can be written as impure and thus won't need the files as inputs. both options are valid.


```
import libraries and functions
create variables and vectors

Print to console:  "(version number, version date)
                    Welcome to FSM Parser & Power Optimizer"

Print to console:  "Parse with power optimization method? (y\n): "

Recieve user input

if (bad input) then: display error message and let user try again

(OPTIONAL) Print to console: "Set amount of CFSM:"      # recommended amount would be usually 2

Open kis2 source file       # use the fancy file opening function that we found
test if file opened correctly
if (error): print to console: "Encountered error while opening file. try again? (y\n): "     #if "yes" then run open command again. if "no" then go back to start.

using the same fancy function: let user choose destination folder and destination file name

use function kisFiles2vhd(source file)

if(user wants optimization) then
    {
        optimizeFSM(source file)            # this function creates cfsm files. i.e.: cfsm1.kis, cfsm2.kis, ...
        kisFiles2vhd([cfsm1, cfsm2, ...])
    }

check if operation was succesful
if (success) then print: "File (dest file name) has been created successfully"
else: "Encountered error during operation."         # if possible add error information to let the user solve the problem.

print: "Would you like to parse another file? (y\n): "
if (y) then jump back to the beginning
if (n) then print: "Goodbye."
```

---
### KIS2 file format (source file):

b   = binary number\
b*  = binary number with don't-care, meaning with '-' symbols\
d   = decimal number\
s   = string

```
(optional empty line)
.i d
.o d
.p d
.s d
.r b (optional)    (might sometimes have a space between .r and b, but not always)
(optional empty line)
b* s s b*
b* s s b*
.
.
.
(optional empty line/s)
```

---
### VHDL file format (destination file)

#### Un-optimized format:
```
import libraries
declare entity
    {
        x input  vector
        y output vector
        reset
        clock
    }
declare architecture
declare state machine type with all the states
{
    syncronous proccess
}
```

#### Optimized format:
```
import libraries
declare entity
    {
        x input  vector
        y output vector
        reset
        clock A
        clock B
        (more clocks if there's more then 2 CFSM)
    }
declare architecture
declare state machine type for each CFSM with only the states that were assigned to it
{
    syncronous proccess A
    syncronous proccess B
    (basically a synch proccess for each CFSM)
}
```
