.PHONY: debug main clean reinstall buildmanual unittest unittestexecute unittestclean install
UnitTestPath	:=$(shell pwd)

LibraryUse=-DLIBRARY_USE_THREAD

clean:
	make clean --makefile=Makefile.build

debug:
	make -j 6 debug --makefile=Makefile.build build_flags="-O0 -g3 -D_DEBUG $(LibraryUse)"

main:
	make -j 6 main  --makefile=Makefile.build build_flags="-O3 $(LibraryUse)"

install:
#	rm -rf /home/apparat/gui/server_bar
#	cp binaries/server_bar /home/apparat/gui/server_bar
#	/home/apparat/kill_mcash

reinstall: clean main install

buildmanual:
	doxygen ./manual/config.ini

unittest: check
	make --directory=./test debug

unittestexecute:
	rm -f /home/apparat/gui/tests
	ln -s "$(UnitTestPath)/test/tests" /home/apparat/gui/tests
	test/binaries/test --gtest_catch_exceptions=1

unittestclean:
	make --directory=./test clean

check-xml:
	rm -rf ./test/check
	mkdir ./test/check
	cppcheck --enable=all -j 6 --xml-version=2 ./source 2> ./test/check/report.xml

check:
	cppcheck --enable=all --template=gcc -j 6 ./source