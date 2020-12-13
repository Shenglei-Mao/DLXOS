default:
	cd makeprocs; make
	cd reaction1; make
	cd reaction2; make
	cd reaction3; make
	cd producer1; make
	cd producer2; make

clean:
	cd makeprocs; make clean
	cd reaction1; make clean
	cd reaction2; make clean
	cd reaction3; make clean
	cd producer1; make clean
	cd producer2; make clean

run:
	cd ../../bin; dlxsim -x os.dlx.obj -a -u makeprocs.dlx.obj 3 12; ee469_fixterminal
