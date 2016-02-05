build=build
mousebox_src=src/mousebox.c
install_dir=/usr/local/bin

clean:
	rm -rf $(build)/*

install: mousebox
	install src/gidoo.sh $(install_dir)/gidoo
	install $(build)/mousebox $(install_dir)/mousebox

mousebox:
	gcc -Wall -Wextra -o $(build)/mousebox $(mousebox_src) -lX11
	chmod +x $(build)/mousebox

dev:
	gcc -Wall -Wextra -o $(build)/gidoo src/main.c src/mousebox.h src/mousebox.c src/arglist.h src/arglist.c -lX11 -std=c99

.PHONY: mousebox install devinstall
