#include<u.h>
#include<libc.h>
#include"acc.c"
#include"fona.h"

#define MAX_DELTA 2.2
#define MIN_DELTA -MAX_DELTA

float
delta(float *x, int count) {
	int i;
	float avg_1 = 0;
	float avg_2 = 0;

	for(i = 0; i<count/2; i++) {
		avg_1 += x[i];
	}

	for(i = 0; i<count; i++) {
		avg_2 += x[i];
	}

	avg_1 = avg_1/((float)(count/2));
	avg_2 = avg_2/((float)(count));

print("avg -> %.6f, %.6f\n", avg_1, avg_2);
	return avg_1/avg_2;
}

int
check_delta(struct emdqueue *s) {
	float x_delta = delta(s->x, s->n_x);
	float y_delta = delta(s->y, s->n_y);
	float z_delta = delta(s->z, s->n_z);
print("%.6f %.6f %.6f\n", x_delta, y_delta, z_delta);
	if(
		(x_delta > MAX_DELTA || x_delta < MIN_DELTA) ||
		(y_delta > MAX_DELTA || y_delta < MIN_DELTA) ||
		(z_delta > MAX_DELTA || z_delta < MIN_DELTA) ||
		s->orientation[1] != s->orientation[0]
	) {
		return 1;
	}

	return 0;
}

void
main(int argc, char **argv) {
	int acc_ctl, acc_data, eia_data, eia_ctl;
	struct emdqueue *emd = init_emdqueue();
	int i, err;

	if(argc !=2 ) {
		print("Please pass a cell phone number as an argument\n");
		return;
	}

	// TODO: Check validity of cell phone number

	/* Initialize accelerometer */
	err = acc_initialize(&acc_data, &acc_ctl, 0x1D);
	if(err < 0) {
		print("Error initializing accelerometer\n");
		return;
	}

	/* Initialize fona */
	init_fona(&eia_data, &eia_ctl);

	for(i=0; ;i++) {
		acc_get_sample(acc_ctl, acc_data, emd);
		if(i>50 && (check_delta(emd) == 1)) {
			break;
		}
	}

print_emdqueue(emd);
	//send_sms(eia_data, argv[1]);
}