debug:
	gcc -Wall -Wextra -pedantic main.c -g -o hcomp

install: debug
	@mv hcomp ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/hcomp

clean:
	@rm ${DESTDIR}${PREFIX}/bin/hcomp
