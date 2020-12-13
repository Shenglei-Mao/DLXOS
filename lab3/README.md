# DLXOS
<br /><br />


## Environment Setup
```bash
echo "export PATH="/home/min/a/ece695x/labs/common/dlxos/bin:$PATH"" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=/home/min/a/ece695x/labs/common/gcc/lib" >> ~/.bashrc
source ~/.bashrc
```
<br />


## One Level
### Compile OS
```bash
mainframer.sh 'cd one-level/os && make'
```
### Compile a particular user program
```bash
mainframer.sh '(cd one-level/apps/example && make)'
```
### Clean OS
```bash
mainframer.sh 'cd one-level/os && make clean'
```
### Clean user program
```bash
mainframer.sh '(cd one-level/apps/example && make clean)'
```
### How to Run User Program 
```bash
cd one-level/os
make run
make run-debug
```
<br />


## Fork
Note: Please Open m flag for PTE comparing! (5 pages initially same after fork! Code area, global area and user stack area are all same, after user call trap with different argument(argument passed by using user stack), the user stack copy on write! Now 4 pages for code area and global area still same but user stack page differ now!)
<br />


## Two Level
### Compile OS
```bash
mainframer.sh 'cd two-level/os && make'
```
### Compile a particular user program
```bash
mainframer.sh '(cd two-level/apps/example && make)'
```
### Clean OS
```bash
mainframer.sh 'cd two-level/os && make clean'
```
### Clean user program
```bash
mainframer.sh '(cd two-level/apps/example && make clean)'
```
### How to Run User Program 
```bash
cd two-level/os
make run
make run-debug
```
<br />


### How to Turn in
```bash
turnin -c ece695x -p lab3 lab3
```
<br />


### One-level/Two-level Paging TestCase
```
1. Print "Hello World" and exit.
2. Access memory beyond the maximum virtual address.
3. Access memory inside the virtual address space, but outside of currently allocated pages.
4. Cause the user function call stack to grow larger than 1 page.
5. Call the "Hello World" program 100 times to make sure you are rightly allocating and freeing pages. (free page function test)
6. Spawn 30 simultaneous processes that print a message, count to a large number in a for loop, and then print another message before exiting. You should choose a number large enough for counting that all 30 processes end up running at the same time. You should not run out of memory with 30 processes.
7. Access memory at another L1 page
```
Note: In two-level paging, testcase covers that case that stack grow more than 64 pages to test L1 page fault as well as L2 page fault.
<br />


### How to switch between testcases?
Change input argument to makeproc at Makefile(./apps/examples) to swith between testcases
```bash
run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj [testcase_number]; ee469_fixterminal
```