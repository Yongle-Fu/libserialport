#include <libserialport.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Example of how to send and receive data.
 *
 * This example file is released to the public domain. */

/* Helper function for error handling. */
int check(enum sp_return result);

int main(int argc, char **argv)
{
	/* The ports we will use. */
	struct sp_port *port;

	/* Open and configure port. */
	check(sp_get_port_by_name("/dev/tty.usbserial-14220", &port));

	printf("Opening port.\n");
	check(sp_open(port, SP_MODE_READ_WRITE));

	printf("Setting port to 115200 8N1, no flow control.\n");
	check(sp_set_baudrate(port, 9600));
	printf("Baudrate done");
	check(sp_set_bits(port, 8));
	check(sp_set_parity(port, SP_PARITY_NONE));
	check(sp_set_stopbits(port, 1));
	check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));

	/* Now send some data on each port and receive it back. */
	struct sp_port *tx_port = port;
	struct sp_port *rx_port = port;

	/* The data we will send. */
	char *data = {0x42,0x6E,0x43,0x50,0x02,0x01,0x00,0x04,0x32,0x02,0x08,0x01,0x34,0xC7,0x4D,0xBA};
	int size = strlen(data);

	/* We'll allow a 1 second timeout for send and receive. */
	unsigned int timeout = 1000;

	/* On success, sp_blocking_write() and sp_blocking_read()
		* return the number of bytes sent/received before the
		* timeout expired. We'll store that result here. */
	int result;

	/* Send data. */
	printf("Sending '%s' (%d bytes).\n", data, size);
	result = check(sp_blocking_write(tx_port, data, size, timeout));

	/* Check whether we sent all of the data. */
	if (result == size)
		printf("Sent %d bytes successfully.\n", size);
	else
		printf("Timed out, %d/%d bytes sent.\n", result, size);

	/* Allocate a buffer to receive data. */
	size = 1024;
	char *buf = malloc(size);

	/* Try to receive the data on the other port. */
	printf("Receiving %d bytes on port %s.\n", size, sp_get_port_name(rx_port));
	result = check(sp_blocking_read(rx_port, buf, size, timeout));

	/* Check whether we received the number of bytes we wanted. */
	if (result == size)
		printf("Received %d bytes successfully.\n", size);
	else
		printf("Timed out, %d/%d bytes received.\n", result, size);

	/* Check if we received the same data we sent. */
	// buf[result] = '\0';
	// printf("Received '%s'.\n", buf);

	/* Free receive buffer. */
	free(buf);

	/* Close ports and free resources. */
	check(sp_close(port));
	sp_free_port(port);

	return 0;
}

/* Helper function for error handling. */
int check(enum sp_return result)
{
	/* For this example we'll just exit on any error by calling abort(). */
	char *error_message;

	switch (result) {
	case SP_ERR_ARG:
		printf("Error: Invalid argument.\n");
		abort();
	case SP_ERR_FAIL:
		error_message = sp_last_error_message();
		printf("Error: Failed: %s\n", error_message);
		sp_free_error_message(error_message);
		abort();
	case SP_ERR_SUPP:
		printf("Error: Not supported.\n");
		abort();
	case SP_ERR_MEM:
		printf("Error: Couldn't allocate memory.\n");
		abort();
	case SP_OK:
	default:
		return result;
	}
}
