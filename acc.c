#include<u.h>
#include<libc.h>

#define ON 1
#define OFF 0

#define i2caddress 0x1D
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

int i2c_write(uchar *buf, int len, int fd);

int
i2c_bind(int *data, int *ctl, uchar addr) {
	char data_path[20];
	char ctl_path[20];
	char driver_name[5];

	sprint(data_path, "/dev/i2c.%x.data\0", addr);
	sprint(ctl_path, "/dev/i2c.%x.ctl\0", addr);
	sprint(driver_name, "#J%x\0", addr);

	*data = open(data_path, ORDWR);
	if(*data < 0) {
		bind(driver_name, "/dev", MAFTER);
		*data = open(data_path, ORDWR);
		if(*data < 0) {
			print("Error opening %s\n", data_path);
			return -1;
		}
	}
	*ctl = open(ctl_path, ORDWR);
	return 0;
}

void
toggle_subaddress(int ctl, int toggle) {
	char *subaddr_mode = calloc(13, sizeof(char));

	sprint(subaddr_mode, "subaddress %d\0", toggle);
	i2c_write((uchar*)subaddr_mode, 13, ctl);
	free(subaddr_mode);
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
i2c_read_register(int fd_data, int fd_ctl, uchar addr, uchar *buf, int nlen) {
	int ret;

	toggle_subaddress(fd_ctl, ON);
	sleep(3);
	ret = pread(fd_data, buf, nlen, addr);
	toggle_subaddress(fd_ctl, OFF);
	sleep(3);

	if(ret != nlen) {
		return -1;
	}
	return 0;
}

int
acc_activate(int acc_data, int acc_ctl) {
	int err;
	uchar temp;
	uchar *buf = calloc(1, sizeof(uchar));

	// Disable FIFO
	/*if(i2c_write(create_i2c_write_buffer(F_SETUP, 0x00), 2, acc_data) != 2) {
		print("Failed to disable FIFO setup\n");
		return -1;
	}*/

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

	if(i2c_write(create_i2c_write_buffer(XYZ_DATA_CFG, 0x01), 2, acc_data) != 2) {
		print("Failed to enable 4g scale range\n");
		return -1;
	}

	// Set high resolution mode on
	if(i2c_write(create_i2c_write_buffer(ctrl_reg2, 1<<1), 2, acc_data) != 2) {
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
	toggle_subaddress(*acc_ctl, OFF);
	err = acc_activate(*acc_data, *acc_ctl);
	if(err < 0) {
		return -1;
	}

	return 0;
}

int
acc_get_sample(int acc_ctl, int acc_data) {
	uchar x_msb, x_lsb, y_msb, y_lsb, z_msb, z_lsb;
	uchar pl_status, xyz_range;
	int read;

	x_msb = 0;

	read = i2c_read_register(acc_data, acc_ctl, 0x01, &x_msb, 1);
	if(read < 0) {
		print("failed read\n");
	} else {
		print("read!: %x\n", x_msb);
	}
	read = i2c_read_register(acc_data, acc_ctl, 0x02, &x_lsb, 1);
	read = i2c_read_register(acc_data, acc_ctl, 0x03, &y_msb, 1);
	read = i2c_read_register(acc_data, acc_ctl, 0x04, &y_lsb, 1);
	read = i2c_read_register(acc_data, acc_ctl, 0x05, &z_msb, 1);
	read = i2c_read_register(acc_data, acc_ctl, 0x06, &z_lsb, 1);
	read = i2c_read_register(acc_data, acc_ctl, plstatus, &pl_status, 1);
	read = i2c_read_register(acc_data, acc_ctl, XYZ_DATA_CFG, &xyz_range, 1);

	print("XYZ: %x %x %x %x %x %x, PL: %x, G: %x\n", x_msb, x_lsb, y_msb, y_lsb, z_msb, z_lsb, pl_status, xyz_range);
	
	return 0;
}

void
main() {
	int acc_ctl, acc_data;
	int i, err;

	print("Initializing\n");
	err = acc_initialize(&acc_data, &acc_ctl, 0x1D);
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