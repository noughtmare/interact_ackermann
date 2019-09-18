all: interact_ackermann interact_ackermann_call_stack interact_ackermann-hs

interact_ackermann_call_stack: interact_ackermann_call_stack.c
	LD_RUN_PATH=/usr/local/lib cc -o interact_ackermann_call_stack interact_ackermann_call_stack.c -L/usr/local/lib -I/usr/local/include -lgjalloc -O3

interact_ackermann: interact_ackermann.c
	LD_RUN_PATH=/usr/local/lib cc -o interact_ackermann interact_ackermann.c -L/usr/local/lib -I/usr/local/include -lgjalloc -O3

interact_ackermann-hs: interact_ackermann.hs
	ghc -o interact_ackermann-hs interact_ackermann.hs -O2
	
clean:
	rm interact_ackermann
	rm interact_ackermann_call_stack
	rm interact_ackermann-hs
