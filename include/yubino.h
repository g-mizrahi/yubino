#ifndef YUBINO_H
#define YUBINO_H

#include <stdint.h>

typedef uint8_t command_t;
typedef uint8_t status_t;

#define COMMAND_LIST_CREDENTIALS 0
#define COMMAND_MAKE_CREDENTIAL 1
#define COMMAND_GET_ASSERTION 2
#define COMMAND_RESET 3

#define STATUS_OK 0
#define STATUS_ERR_COMMAND_UNKOWN 1
#define STATUS_ERR_CRYPTO_FAILED 2
#define STATUS_ERR_BAD_PARAMETER 3
#define STATUS_ERR_NOT_FOUND 4
#define STATUS_ERR_STORAGE_FULL 5
#define STATUS_ERR_APPROVAL 6

status_t recv_command(uint8_t[41]);
status_t exec_command(uint8_t[41]);
void send_error_message(status_t *);

#endif /*YUBINO_H*/