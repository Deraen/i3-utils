CFLAGS=-Wall `pkg-config --libs --cflags i3ipc-glib-1.0 json-glib-1.0`

i3-window-exists: i3-window-exists.c
	gcc -g -o $@ $? ${CFLAGS}
