.DEFAULT:
	make -j 6 $(MAKECMDGOALS) --makefile=Makefile.build build_flags="-O0 -g3 -D_DEBUG -DUNITTEST"

release:
	make -j 6 main --makefile=Makefile.build build_flags="-O2 -fomit-frame-pointer"


