# dlxos

## How to Compile (Order Matter)
mainframer.sh 'cd os && make'<br/>
mainframer.sh 'cd apps && make'

## How to Clean
mainframer.sh 'cd apps && make clean'<br/>
mainframer.sh 'cd os && make clean'

## How to Run User Program
dlxsim -x ~/ece695/lab1/os/work/os.dlx.obj -a -u ~/ece695/lab1/apps/work/userprog.dlx.obj
