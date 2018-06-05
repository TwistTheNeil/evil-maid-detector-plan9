#include<u.h>
#include<libc.h>

#define MAX_DELTA 0.5 // Change this later

struct emdstack {
	int x[2];
	int y[2];
	int z[2];
	int orientation[2];
};

int
delta(int x[2]) {
	return abs(x[1] - x[0]);
}

int
check_delta(struct emdstack s) {
	if(
		delta(s.x) > MAX_DELTA ||
		delta(s.y) > MAX_DELTA ||
		delta(s.z) > MAX_DELTA ||
		s.orientation[1] != s.orientation[0]
	) {
		return 1;
	}

	return 0;
}

int main() {
	return 0;
}