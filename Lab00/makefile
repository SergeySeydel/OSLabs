all: myls

myls: main.c
	gcc $< -o $@

clean: myls
	rm $<

gdb:main.c
	gcc -g $< -o $@