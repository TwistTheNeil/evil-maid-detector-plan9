#include<u.h>
#include<libc.h>

#define i2caddress 0x1D
#define ctrl_reg1 0x2A
#define ctrl_reg2 0x2B
#define ctrl_reg4 0x2D
#define ctrl_reg5 0x2E
#define whoami 0x0D
#define plcfg 0x11
#define pldebounce 0x12
#define deviceid 0x1A
#define XYZ_DATA_CFG 0x0E

int
acc_bind(int *data, int *ctl) {
	*data = open("/dev/i2c.1d.data", ORDWR);
	if(*data < 0) {
		bind("#J1d", "/dev", MAFTER);
		*data = open("/dev/i2c.1d.data", ORDWR);
		if(*data < 0) {
			print("Error opening /dev/i2c.1d.data\n");
			return -1;
		}
	}
	*ctl = open("/dev/i2c.1d.ctl", ORDWR);
	return 0;
}

int
acc_subaddress_off(int ctl) {
	char subaddr_0[13] = "subaddress 0\0";
	if(pwrite(ctl, subaddr_0, strlen(subaddr_0), 0) != strlen(subaddr_0)) {
		print("Failed to turn off subaddressing\n");
		return -1;
	}
	return 0;
}

int
acc_subaddress_on(int ctl) {
	char subaddr_1[13] = "subaddress 1\0";
	if(pwrite(ctl, subaddr_1, strlen(subaddr_1), 0) != strlen(subaddr_1)) {
		print("Failed to turn on subaddressing\n");
		return -1;
	}
	return 0;
}

int
i2c_write(uchar *buf, int len, int fd) {
	int written = pwrite(fd, buf, len, 0);

	/* Experimentally determined
		Seems like the register needs some time to
		write a byte
	*/
	sleep(3);

	return written;
}

uchar*
create_i2c_write_buffer(uchar addr, uchar instr) {
	uchar *buf = calloc(2, sizeof(uchar));
	buf[0] = addr;
	buf[1] = instr;
	return buf;
}

int
acc_read_register(int fd_data, int fd_ctl, uchar addr, uchar *buf) {
	int ret;

	acc_subaddress_on(fd_ctl);
	ret = pread(fd_data, buf, 1, addr);
	acc_subaddress_off(fd_ctl);

	if(ret != 1) {
		return -1;
	}
	return 0;
}

int
acc_activate(int acc_data, int acc_ctl) {
	int err;
	uchar temp;
	uchar *buf = calloc(1, sizeof(uchar));

	print("Turning on accelerometer\n");
	err = acc_read_register(acc_data, acc_ctl, ctrl_reg1, buf);
	if(err < 0) {
		print("Failed to read ctrl_reg1\n");
		return -1;
	}
	if(((buf[0]<<7)>>7) != 0x01) {
		if(i2c_write(create_i2c_write_buffer(ctrl_reg1, (0x01 | (1<<2))), 2, acc_data) != 2) {
			print("Failed to turn on accelerometer\n");
			return -1;
		}
		print("Accelerometer is on!\n");
	} else {
		print("Accelerometer is already on.\n");
	}


	print("Double checking device ID");
	err = acc_read_register(acc_data, acc_ctl, whoami, buf);
	if(err < 0) {
		print("Failed reading ctrl register 1\n");
		return -1;
	}
	if(buf[0] != deviceid) {
		print(" - Failed to turn on accelerometer (0x00)\n");
		return -1;
	}
	print(" - Device ID is correct!\n");

	// Reset the accelerometer
	if(i2c_write(create_i2c_write_buffer(ctrl_reg2, 0x40), 2, acc_data) != 2) { //change 40 to 1<<6
		print("Failed to reset accelerometer\n");
		return -1;
	}
	sleep(300);

	if(i2c_write(create_i2c_write_buffer(XYZ_DATA_CFG, 0x01), 2, acc_data) != 2) {
		print("Failed to enable 4g scale range\n");
		return -1;
	}

	// Set high resolution mode on
	if(i2c_write(create_i2c_write_buffer(ctrl_reg2, 1<<2), 2, acc_data) != 2) {
		print("Failed to set high resolution\n");
		return -1;
	}

	// Set interrupts on
	if(i2c_write(create_i2c_write_buffer(ctrl_reg4, 0x01), 2, acc_data) != 2) {
		print("Failed to turn on interrupt registers\n");
		return -1;
	}
	if(i2c_write(create_i2c_write_buffer(ctrl_reg5, 0x01), 2, acc_data) != 2) {
		print("Failed to turn on interrupts\n");
		return -1;
	}

	// Portrait/Landscape configuration
	if(i2c_write(create_i2c_write_buffer(plcfg, 1<<6), 2, acc_data) != 2) {
		print("Failed to enable portrait/landscape detection\n");
		return -1;
	}


	return 0;
}

int
acc_initialize(int *acc_data, int *acc_ctl) {
	int err;

	print("Binding i2c interface\n");
	err = acc_bind(acc_data, acc_ctl);
	if(err < 0) {
		return -1;
	}

	print("Activating accelerometer\n");
	acc_subaddress_off(*acc_ctl);
	err = acc_activate(*acc_data, *acc_ctl);
	if(err < 0) {
		return -1;
	}

	return 0;
}

int
acc_get_sample(int acc_ctl, int acc_data) {
	uchar x_msb, x_lsb, y_msb, y_lsb, z_msb, z_lsb;
	int read;

	read = acc_read_register(acc_data, acc_ctl, 0x01, &x_msb);
	read = acc_read_register(acc_data, acc_ctl, 0x02, &x_lsb);
	read = acc_read_register(acc_data, acc_ctl, 0x03, &y_msb);
	read = acc_read_register(acc_data, acc_ctl, 0x04, &y_lsb);
	read = acc_read_register(acc_data, acc_ctl, 0x05, &z_msb);
	read = acc_read_register(acc_data, acc_ctl, 0x06, &z_lsb);

	print("OMG %x %x %x %x %x %x\n", x_msb, x_lsb, y_msb, y_lsb, z_msb, z_lsb);
	return 0;
}

void
main() {
	int acc_ctl, acc_data;
	int i, err;

	print("Initializing\n");
	err = acc_initialize(&acc_data, &acc_ctl);
	if(err < 0) {
		print("Error initializing accelerometer\n");
		return;
	}

	for(i=0; ;i++) {
		if(acc_get_sample(acc_ctl, acc_data) == 1) {
			break;
		}
	}
}