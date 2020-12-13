# DLXOS

## Environment Setup
```bash
echo "export PATH="/home/min/a/ece695x/labs/common/dlxos/bin:$PATH"" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=/home/min/a/ece695x/labs/common/gcc/lib" >> ~/.bashrc
source ~/.bashrc
```

## Compile OS
```bash
mainframer.sh 'cd os && make'
```

## Compile a particular user program
```bash
mainframer.sh '(cd apps/example && make)'
mainframer.sh '(cd apps/q2 && make)'
mainframer.sh '(cd apps/q3 && make)'
mainframer.sh '(cd apps/q4 && make)'
mainframer.sh '(cd apps/q5 && make)'
```

## Clean OS
```bash
mainframer.sh 'cd os && make clean'
```

## Clean user program
```bash
mainframer.sh '(cd apps/example && make clean)'
mainframer.sh '(cd apps/q2 && make clean)'
mainframer.sh '(cd apps/q3 && make clean)'
mainframer.sh '(cd apps/q4 && make clean)'
mainframer.sh '(cd apps/q5 && make clean)'
```

## How to Run User Program 
```bash
cd os
make run
```

## How to Turn in
```bash
turnin -c ece695x -p lab2 lab2
```
