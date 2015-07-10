nothing:

waf:
	wget http://www.freehackers.org/~tnagy/release/waf-1.8.7
	mv waf-1.8.7 waf
	python waf --version
	chmod +x waf

update:
	./waf build
	./waf install

all:
	./waf distclean
	./waf configure --prefix=${HOME}/local-dev
	./waf build
	./waf install

debug:
	./waf distclean
	./waf configure --debug --prefix=${HOME}/local-dev
	./waf -v build
	./waf install

verbose:
	./waf distclean
	./waf configure --prefix=${HOME}/local-dev
	./waf -v build
	./waf install

test:
	TILETREE_ROOT="${HOME}/projects/tiletree" ./build/src/tiletree
