all:	peer tracker

peer: peer.c
	gcc -pthread -Wall $< -o $@

tracker: tracker.c
	gcc -pthread -Wall $< -o $@

clean:
	rm -f peer tracker *.o *~ core

