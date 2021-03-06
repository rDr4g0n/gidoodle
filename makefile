build=build
mousebox_src=src/mousebox.c
install_dir=/usr/local/bin

.DEFAULT_GOAL := compile

create_build_dir:
	mkdir -p $(build)

clean:
	rm -rf $(build)/*

install: compile
	install $(build)/gidoo $(install_dir)/gidoo

compile: create_build_dir
	gcc -Wall -Wextra -Wno-deprecated-declarations \
		-o $(build)/gidoo src/main.c \
		src/debug.h src/debug.c \
		src/config.h src/config.c \
		src/gidoo.h src/gidoo.c \
		src/mousebox.h src/mousebox.c \
		src/arglist.h src/arglist.c \
		-lX11 -std=gnu99

.PHONY: clean install compile create_build_dir
