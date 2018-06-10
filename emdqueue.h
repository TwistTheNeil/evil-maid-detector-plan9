#define QUEUE_MAX 700

struct emdqueue {
	float *x;
	float avg_x;
	int n_x;

	float *y;
	float avg_y;
	int n_y;

	float *z;
	float avg_z;
	int n_z;

	uchar *orientation;
	int n_o;
};

struct emdqueue*
init_emdqueue(void) {
	struct emdqueue *e = calloc(1, sizeof(struct emdqueue));
	e->x = calloc(QUEUE_MAX, sizeof(float));
	e->y = calloc(QUEUE_MAX, sizeof(float));
	e->z = calloc(QUEUE_MAX, sizeof(float));
	e->orientation = calloc(2, sizeof(uchar));

	e->avg_x = 0;
	e->avg_y = 0;
	e->avg_z = 0;

	e->n_x = 0;
	e->n_y = 0;
	e->n_z = 0;
	e->n_o = 0;

	return e;
}

/*
   when adding an element to the queue, we will
   remove the first if the new total exceeds the
   maximum capacity
*/
void
emds_squeeze(float **q, float e, int *count) {
	int i = 0;

	for(i=0; (*count) == QUEUE_MAX && i<(*count)-1; i++) {
		(*q)[i] = (*q)[i+1];
	}

	if (*count < QUEUE_MAX) {
		 (*count)++;
	}

	(*q)[(*count)-1] = e;
}

void
emdq_push(struct emdqueue *s, float x, float y, float z, uchar orientation) {
	emds_squeeze(&s->x, x, &(s->n_x));
	emds_squeeze(&s->y, y, &(s->n_y));
	emds_squeeze(&s->z, z, &(s->n_z));

// TODO: make a generic to handle uchar?
	s->orientation[0] = s->orientation[1];
	s->orientation[1] = orientation;
	s->n_o == 2 ? s->n_o = 2 : s->n_o++;
	
}

void
print_emdqueue(struct emdqueue *s) {
	int i;

	print("X[%d] = [", s->n_x);
	for(i=0;i<s->n_x;i++) {
		print("%.6f, ", s->x[i]);
	}
	print("]\n");

	print("Y[%d] = [", s->n_y);
	for(i=0;i<s->n_y;i++) {
		print("%.6f, ", s->y[i]);
	}
	print("]\n");

	print("Z[%d] = [", s->n_z);
	for(i=0;i<s->n_z;i++) {
		print("%.6f, ", s->z[i]);
	}
	print("]\n");

	print("O[%d] = [", s->n_o);
	for(i=0;i<s->n_o;i++) {
		print("%x, ", s->orientation[i]);
	}
	print("]\n");
}