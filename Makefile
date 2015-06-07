CFLAGS=-Wall `pkg-config --libs --cflags json-glib-1.0`

all: i3-window-exists i3-switch-all

i3-window-exists: i3-window-exists.c
	gcc -g -o $@ $? ${CFLAGS}

i3-switch-all: i3-switch-all.c
	gcc -g -o $@ $? ${CFLAGS}
