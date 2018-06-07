#include<u.h>
#include<libc.h>
#include"acc.c"

#define MAX_DELTA 0.5 // Change this later

int
delta(float x[2]) {
	return abs(x[1] - x[0]);
}

int
check_delta(struct emdstack s) {
	if(
		delta(s.x) > MAX_DELTA ||
		delta(s.y) > MAX_DELTA ||
		//delta(s.z) > MAX_DELTA ||
		s.orientation[1] != s.orientation[0]
	) {
		return 1;
	}

	return 0;
}

void
main() {
	int acc_ctl, acc_data;
	struct emdstack emd;
	int i, err;

	print("Initializing\n");
	err = acc_initialize(&acc_data, &acc_ctl, 0x1D);
	if(err < 0) {
		print("Error initializing accelerometer\n");
		return;
	}

	for(i=0; ;i++) {
		acc_get_sample(acc_ctl, acc_data, &emd);
print("(%.6f, %.6f, %.6f)\n", emd.x[1], emd.y[1], emd.z[1]);
		if(i>2 && (check_delta(emd) == 1)) {
			break;
		}
	}
}