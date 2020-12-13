default:
	cd makeprocs; make
	cd hello_world; make
	cd testcase2; make
	cd testcase3; make
	cd testcase4; make
	cd hello_world_count; make

clean:
	cd makeprocs; make clean
	cd hello_world; make clean
	cd testcase2; make clean
	cd testcase3; make clean
	cd testcase4; make clean
	cd hello_world_count; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 4; ee469_fixterminal

run-debug:
	cd ../../bin; dlxsim -D m -x os.dlx.obj -a -D m -u makeprocs.dlx.obj 5; ee469_fixterminal