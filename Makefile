CFLAGS  = -std=c++11 -Wall -Wextra -Wpedantic -D_FORTIFY_SOURCE=2
LDFLAGS = -lX11

all: stackwm

stackwm: stackwm.cc
	${CXX} -o $@ ${CFLAGS} ${LDFLAGS} $^

format: *.cc
	clang-format -i -style=google $^

clean:
	rm -f stackwm
