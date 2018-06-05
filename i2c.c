#define ON 1
#define OFF 0

int i2c_write(uchar *buf, int len, int fd);
int i2c_bind(int *data, int *ctl, uchar addr);
int i2c_read_register(int fd_data, int fd_ctl, uchar addr, uchar *buf, int nlen);
void toggle_subaddress(int ctl, int toggle);
uchar* create_i2c_write_buffer(uchar addr, uchar instr);

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