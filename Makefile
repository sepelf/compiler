
toy: toy.cc
	g++ -g -std=c++11 $< -o $@

test:
	echo "\033[32mstart test\033[0m"
	echo "def foo(x y) x+foo(y, 4.0);" | ./toy
	echo "\033[32mstart test\033[0m"
	echo "def foo(x y) x+y y;" | ./toy

clean:
	rm -rf toy
