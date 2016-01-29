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

.PHONY: mousebox install devinstall
