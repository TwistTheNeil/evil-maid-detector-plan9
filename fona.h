void
init_fona(int *eia_data, int *eia_ctl) {
	*eia_data = open("/dev/eia0", ORDWR);
	*eia_ctl = open("/dev/eia0ctl", ORDWR);

	fprint(*eia_ctl, "b115200\n");
	fprint(*eia_ctl, "l8 pn s1\n");
}

void
set_text_mode(int eia_data) {
	fprint(eia_data, "AT+CMGF=1\n");
}

void
send_sms(int eia_data, char *number) {
	char wbuf[25];
	char wmsg[25];
	char ctrlz[2];

	/* We need to send ctrl+z when finished typing message */
	sprint(ctrlz, "%c", 0x1A);
	ctrlz[1] = 0;

	/* Set text mode and send message */
	set_text_mode(eia_data);
	sprint(wbuf, "AT+CMGS=\"%s\"\r\n", number);
	sprint(wmsg, "Detected motion\r\n");

	fprint(eia_data, wbuf);
	sleep(5000);
	fprint(eia_data, wmsg);
	fprint(eia_data, ctrlz);
}