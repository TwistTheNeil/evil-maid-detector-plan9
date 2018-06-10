#include<u.h>
#include<libc.h>

void
main(int argc, char **argv) {
	char wbuf[10];
	char wmsg[25];
	char ctrlz[2];
	int eia_data = open("/dev/eia0", ORDWR);
	int eia_ctl = open("/dev/eia0ctl", ORDWR);
	int eia_status = open("/dev/eia0status", ORDWR);

	if(argc !=2 ) {
		print("Please pass a cell phone number as an argument\n");
		return;
	}

	fprint(eia_ctl, "b115200\n");
	fprint(eia_ctl, "l8 pn s1\n");
	sprint(ctrlz, "%c", 0x1A);
	ctrlz[1] = 0;

	//fprint(eia_data, "AT\r\n");
	//read(eia_data, buf, 1);
	//buf[1]=0;
	//print("RETURNED: %s\n", buf);

	sprint(wbuf, "AT+CMGS=\"%s\"\r\n", argv[1]);
	sprint(wmsg, "Detected motion\r\n");

	fprint(eia_data, wbuf);
	sleep(5000);
	fprint(eia_data, wmsg);
	fprint(eia_data, ctrlz);
}