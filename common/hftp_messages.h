/****************************************************************************
 * hftp_messages.h
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
 *
 * HFTP message structures to match the message structure size in 
 * upd_sockets.h
****************************************************************************/

#define MAX_FILENAME_SIZE 1444
#define MAX_DATA_LENGTH 1468
#define INITMSG 1       // Initialization message type
#define TERMMSG 2       // Termination message type
#define DATAMSG 3       // Data message type
#define RESPONSEMSG 255 // Response message type
#define RESPONSELEN 4   // Length of a response message (4 bytes)
#define RESPONDVALID 0  // Error code of a response to a valid request
#define RESPONDINVALID 1 // Error code of a response to an invalid request
#define TOKEN_LENGTH 16 // Length of authentication tokens

// Struct for an HFTP control message
typedef struct {
  int length;             // Length of message - not actually sent, but part
                          // of message struct
  uint8_t type;           // Message type (1 byte)
  uint8_t sequence;       // Message sequence number (1 bytes)
  uint16_t filename_len;  // Length of filename (2 bytes)
  uint32_t file_size;     // File size (4 bytes)
  uint32_t checksum;      // File checksum (4 bytes)
  uint8_t token[TOKEN_LENGTH];         // AUTH token (16 bytes)
  uint8_t filename[MAX_FILENAME_SIZE]; // Filename buffer (1444 bytes)
} hftp_control_message;

// Struct for an HFTP data message
typedef struct {
  int length;             // Length of message - not actually sent, but part
                          // of message struct
  uint8_t type;           // Message type (1 byte)
  uint8_t sequence;       // Message sequence number (1 bytes)
  uint16_t data_length;   // Length of data field (2 bytes)
  uint8_t data[MAX_DATA_LENGTH]; // Data buffer (1468 bytes)
} hftp_data_message;

// Struct for an HFTP response message
typedef struct {
  int length;             // Length of message - not actually sent, but part
                          // of message struct
  uint8_t type;           // Message type (1 byte)
  uint8_t sequence;       // Message sequence number (1 bytes)
  uint16_t error_code;    // Error code (2 bytes)
  uint8_t padding[1468];  // Padding to match message size (1468)
} hftp_response_message;