all:	peer tracker

peer: peer.c
	gcc -Wall $< -o $@

tracker: tracker.c
	gcc -Wall $< -o $@

clean:
	rm -f peer tracker *.o *~ core

