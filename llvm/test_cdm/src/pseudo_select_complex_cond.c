int foo(int a, int b, int c, int d){
	return (a < b) && (c == d) && (b >= c);
}

int bar(int a, int b, int c, int d){
	if (foo(d, c, b, a)){
		return 1;
	}
	return 228;
}

int main(){
	return bar(5, 5, 6, 1);
}
