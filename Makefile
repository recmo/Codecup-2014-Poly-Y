
polio: main.cpp
	g++ -std=c++11 -DLOCAL -O2 -g main.cpp -o polio -Wall -Wextra -Wno-unused-parameter -Werror=return-type -Werror=switch -lm

competition: polio
	cd caia/bin; ./caiaio -m competition

