#include "i2c.h"
#include "emdqueue.h"

#define i2caddress 0x1D
#define OUT_REG 0x01
#define ctrl_reg1 0x2A
#define ctrl_reg2 0x2B
#define ctrl_reg4 0x2D
#define ctrl_reg5 0x2E
#define whoami 0x0D
#define plcfg 0x11
#define pldebounce 0x12
#define plstatus 0x10
#define deviceid 0x1A
#define XYZ_DATA_CFG 0x0E
#define F_SETUP 0x09
#define DIVIDER_4G 2048

float GRAVITY = 9.8;

int
acc_activate(int acc_data, int acc_ctl) {
	int err;
	uchar temp;
	uchar *buf = calloc(1, sizeof(uchar));

	print("Double checking device ID");
	err = i2c_read_register(acc_data, acc_ctl, whoami, buf, 1);
	if(err < 0) {
		print("\nFailed reading ctrl register 1\n");
		return -1;
	}
	if(buf[0] != deviceid) {
		print("\nFailed to turn on accelerometer (0x00)\n");
		return -1;
	}
	print(" - Device ID is correct!\n");

	// Reset the accelerometer
	if(i2c_write(create_i2c_write_buffer(ctrl_reg2, 1<<6), 2, acc_data) != 2) {
		print("Failed to reset accelerometer\n");
		return -1;
	}
	err = i2c_read_register(acc_data, acc_ctl, ctrl_reg2, &temp, 1);
	while( (temp & (1<<6) ) == 1) {
		err = i2c_read_register(acc_data, acc_ctl, ctrl_reg2, &temp, 1);
		sleep(300);
	}

	// Disable FIFO
	if(i2c_write(create_i2c_write_buffer(F_SETUP, 0x00), 2, acc_data) != 2) {
		print("Failed to disable FIFO setup\n");
		return -1;
	}

	// Enable 4G range
	if(i2c_write(create_i2c_write_buffer(XYZ_DATA_CFG, 0x01), 2, acc_data) != 2) {
		print("Failed to enable 4g scale range\n");
		return -1;
	}

	// Set high resolution mode on
	if(i2c_write(create_i2c_write_buffer(ctrl_reg2, 1<<1), 2, acc_data) != 2) {
		print("Failed to set high resolution\n");
		return -1;
	}

	// Portrait/Landscape configuration
	if(i2c_write(create_i2c_write_buffer(plcfg, 1<<6), 2, acc_data) != 2) {
		print("Failed to enable portrait/landscape detection\n");
		return -1;
	}

	// activate with low noise mode
	print("Turning accelerometer on\n");
	if(i2c_write(create_i2c_write_buffer(ctrl_reg1, (0x01|(1<<2))), 2, acc_data) != 2) {
		print("Failed to enable accelerometer with low noise mode\n");
		return -1;
	}

	return 0;
}

int
acc_initialize(int *acc_data, int *acc_ctl, uchar addr) {
	int err;

	print("Binding i2c interface\n");
	err = i2c_bind(acc_data, acc_ctl, addr);
	if(err < 0) {
		return -1;
	}

	print("Activating accelerometer\n");
	err = acc_activate(*acc_data, *acc_ctl);
	if(err < 0) {
		return -1;
	}

	return 0;
}

int
acc_get_sample(int acc_ctl, int acc_data, struct emdqueue *emd) {
	char acc_out[6];
	uchar pl_status;
	int read;

	read = i2c_read_int_register(acc_data, acc_ctl, OUT_REG, acc_out, 6);
	if(read < 0) {
		print("Error reading 6 bytes from registers\n");
		return -1;
	}

	read = i2c_read_register(acc_data, acc_ctl, plstatus, &pl_status, 1);
	if(read < 0) {
		print("Error reading P/L status\n");
		return -1;
	}

	emdq_push(	emd,
			(float)((short)(((acc_out[0]<<8) | (acc_out[1]))>>2))/DIVIDER_4G*GRAVITY,
			(float)((short)(((acc_out[2]<<8) | (acc_out[3]))>>2))/DIVIDER_4G*GRAVITY,
			(float)((short)(((acc_out[4]<<8) | (acc_out[5]))>>2))/DIVIDER_4G*GRAVITY,
			pl_status
	);

	return 0;
}