# DLXOS
<br /><br />


## Environment Setup
```bash
echo "export PATH="/home/min/a/ece695x/labs/common/dlxos/bin:$PATH"" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=/home/min/a/ece695x/labs/common/gcc/lib" >> ~/.bashrc
source ~/.bashrc
```
<br />


## Flat
### Compile OS
```bash
mainframer.sh 'cd flat/os && make'
```
### Compile a particular user program
```bash
mainframer.sh '(cd flat/apps/fdisk && make)'
mainframer.sh '(cd flat/apps/ostests && make)'
mainframer.sh '(cd flat/apps/files && make)'
```
### Clean OS
```bash
mainframer.sh 'cd flat/os && make clean'
```
### Clean user program
```bash
mainframer.sh '(cd flat/apps/fdisk && make clean)'
mainframer.sh '(cd flat/apps/ostests && make clean)'
mainframer.sh '(cd flat/apps/files && make clean)'
```
### How to Run User Program 
```bash
cd flat/os
make run
make run-debug
```
<br />


### How to Turn in
```bash
turnin -c ece695x -p lab4 lab4
```


### How to use ostest.c / Kernel Level Testing
```bash
make fdisk
test all dfs layer (comment other layer test code)

make fdisk
comment all dfs layer testing code (comment other layer test code)

make fdisk
test all inode and file system layer (comment other layer test code)
```
Why we want to test each layer seperately?
At each layer, we deliberately run code that break things, for example, we deliberately free and force wipe out (clean) the data by give a designated physical block, this could break things between inode and block relationship, so we recomment test each layer seperately.
<br />

### User Program Level Testing
Simplely run user prog (files.c), yes, which is a very bad file name (duplicate with one files in kernel). And you should be able to see the out put.


### make command
make fdisk (give you a brand new disk)
make run (run kernel testing ostest.c)
make files (run user level files.c)


### What is done and what is not done
fdisk, dfs layer, inode layer and file layer are well implemented and tested
however, multilevel layer is not implemented due to limited amount of time