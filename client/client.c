/****************************************************************************
 * client.c
 *
 * Computer Science 3357a - Fall 2015
 * Author: Jerridan Quiring
****************************************************************************/

#include "client.h"

// Main function
int main(int argc, char** argv) {
  //Open call to syslog
  openlog("hooli_client", LOG_PERROR | LOG_PID | LOG_NDELAY, LOG_USER);

  // Optional arguments and defaults
  char* hostname = "localhost";     // HMDS server hostname
  char* hooli_dir;                  // Hooli root directory
  asprintf(&hooli_dir, "%s/hooli/", getenv("HOME")); // Path to "~/hooli/"
  char* port = "9000";              // Server's port
  int verbose_flag = 0;             // Verbose output flag
  char* hftp_hostname = "localhost";// HFTP server hostname
  char* hftp_port = "10000";        // HFTP server port

  // Required arguments
  char* username = NULL;            // Client's username
  char* password = NULL;            // Client's password

  handleOptions(argc, argv, &hostname, &port, &hooli_dir, &verbose_flag, 
    &hftp_hostname, &hftp_port);

  // Set syslog level
  setlogmask(LOG_UPTO(verbose_flag ? LOG_DEBUG : LOG_INFO));

  handleCredentials(argc, argv, &username, &password);

  logArguments(username, password, hostname, hooli_dir, port, verbose_flag, 
    hftp_hostname, hftp_port);

  // Scan root Hooli directory
  hooli_file* file = handleHooliScan(hooli_dir);

  // If there are were no files in the Hooli directory, exit
  if(!file) {
    closelog();
    free(hooli_dir);
    exit(EXIT_SUCCESS);
  }

  // Initialize socket file descriptor
  int sockfd = -1;

  // Authenticate the user and get a token
  char* token = handleServerAuth(hostname, port, username, password, &sockfd);

  // If authentication failed, exit
  if(NULL == token) {
    freeHooliFileList(file);
    free(hooli_dir);
    exit(EXIT_FAILURE);
  }

  // Get the list of files to be uploaded
  char* upload_list = getUploadList(sockfd, token, file);
  if(NULL == upload_list) {
    free(token);
    freeHooliFileList(file);
    free(hooli_dir);
    closelog();
    exit(EXIT_FAILURE);
  }
  
  // Copy the upload list and convert it into a list of hooli files
  char* upload_list_cpy;
  asprintf(&upload_list_cpy, "%s", upload_list);
  int numfiles = 0; // Number of files being uploaded
  hooli_file* uploads = convertUploadList(upload_list_cpy, file, &numfiles);
  free(upload_list_cpy);

  if(uploads) {
    // Log requested files
    logRequestedFiles(upload_list);

    // Upload files to HFTP server
    int uploaded = handleFileUploads(hooli_dir, uploads, token, hftp_hostname, hftp_port, numfiles);
    syslog(LOG_INFO, "%d files uploaded to server", uploaded);
  }

  free(upload_list);
  free(token);
  free(hooli_dir);
  freeHooliFileList(uploads);
  closelog();
  exit(EXIT_SUCCESS);
}

// Converts the upload list received from HMDP to a hooli_file structure
// Uses the original file list, copying over to new list
hooli_file* convertUploadList(char* upload_list, hooli_file* old_list, 
  int* numfiles) 
{
  char* body = strstr(upload_list, "\n\n"); // Get pointer to body
  body = body + 2;                          // Put pointer ahead of /n/n
  char* upload = NULL;                      // One requested file
  const char separator[2] = "\n";           // Char separating files in list

  // Point to first node in old file list
  hooli_file* current_old = old_list;

  // Create first node for new file list
  hooli_file* new_list = NULL;
  hooli_file* current_new = NULL;
  int count = 0; // Counter for building new list

  upload = strtok(body, separator);
  while(NULL != upload) {
    // Find the first node in the old file list that matches the current
    // upload's filepath
    while(0 != strcmp(upload, current_old->filepath)) {
      current_old = current_old->next;
    }

    // Point current node of new list accordingly
    if(count == 0) {
      new_list = malloc(sizeof(hooli_file));
      current_new = new_list;
    } else {
      current_new->next = malloc(sizeof(hooli_file));
      current_new = current_new->next;
    }

    // Assign the filepath and checksum to the new list
    // Create copies of filepath so that we can later free the old list
    char* filepath_copy; // Temp pointer for copying filepath
    asprintf(&filepath_copy, "%s", current_old->filepath);
    current_new->filepath = filepath_copy;
    current_new->checksum = current_old->checksum;
    current_new->next = NULL;

    // Get next upload
    upload = strtok(NULL, separator);
    *numfiles = *numfiles + 1;
    count++;
  }
  freeHooliFileList(old_list);
  return new_list;
}

// Logs requested files from a 302 server response
void logRequestedFiles(char* upload_list) {
  syslog(LOG_INFO, "Server requested the following files:");
  char* body = strstr(upload_list, "\n\n"); // Get pointer to body
  body = body + 2;                          // Put pointer ahead of /n/n
  char* file = NULL;                        // One requested file
  const char separator[2] = "\n";           // Char separating files in list

  // Log files in list
  file = strtok(body, separator);
  while(NULL != file) {
    syslog(LOG_INFO,"* %s", file);
    file = strtok(NULL, separator);
  }
}

// Handles the scanning of the Hooli directory
// Returns a linked list of Hooli files
hooli_file* handleHooliScan(char* hooli_dir) {
  hooli_file* file = malloc(sizeof(hooli_file)); // Empty hooli_file node
  hooli_file* first = file;                      // First hooli_file in list
  int file_count = 0; // Number of files scanned in Hooli directory
  syslog(LOG_INFO, "Scanning Hooli directory: %s", hooli_dir);
  scanHooliDir(hooli_dir, strlen(hooli_dir), &file, &file_count);
  syslog(LOG_DEBUG, "# files scanned: %d", file_count);
  file = first;       // Point back to first node in hooli_file list

  if(file_count > 0) {
    logHooliFiles(file);
    return file;
  } else {
    syslog(LOG_INFO, "No files found in Hooli directory");
    free(file);
    return NULL;
  }
}

// Frees all memory allocated to a linked list of Hooli files
void freeHooliFileList(hooli_file* file) {
  hooli_file* current = file; // Current Hooli file
  hooli_file* next;           // Pointer to next Hooli file
  while(current) {
    next = current->next;
    free(current->filepath);
    free(current);
    current = next;
  }
}

// Logs all Hooli files in the linked list in debug mode
void logHooliFiles(hooli_file* file) {
  while(file) {
    syslog(LOG_DEBUG, "* %s (%X)", file->filepath, file->checksum);
    file = file->next;
  }
}

// Recursively scans a specified directory, calculates file checksums, and
// adds the entries to a linked list
void scanHooliDir(char *dir, const int base_path_length, hooli_file** file, 
  int* count) 
{
  DIR* dirp;         // Pointer to directory stream
  struct dirent* dp; // Pointer to current object in directory stream

  // Open directory
  if(NULL == (dirp = opendir(dir))) {
    // If the directory can't be opened, log an error and move on
    syslog(LOG_WARNING, "Unable to open directory '%s'", dir);
    return;
  }

  // Iterate over files/folders in directory
  while(NULL != (dp = readdir(dirp))) {
    // Don't scan directories like '.' or '..'
    if(isCurrentOrParentDir(dp->d_name)) {
      continue;
    }

    // Scan subdirectories recursively
    if(DT_DIR == dp->d_type) {
      // Concatenate parent directory and current directory names
      char* current_path = getFullPathName(dir, dp->d_name, true);
      scanHooliDir(current_path, base_path_length, file, count);
      free(current_path);
    } else {
      // Concatenate parent directory and current directory names
      char* current_path = getFullPathName(dir, dp->d_name, false);
      // int file_length; // Length of file in bytes
      // char* buffer;    // Contents of file
      // readFileToBuffer(current_path, &buffer, &file_length);
      int checksum = calcChecksum(current_path);
      if(-1 != checksum) {
        // uLong checksum = calcChecksum(buffer, file_length); // checksum
        // Copy relative path to new string
        char* filepath = getStrCopy(current_path + base_path_length);
        // Add hooli_file entry
        addHooliFile(checksum, filepath, file, *count);
        (*count)++;
        // free(buffer);
      }
      free(current_path);
    }
  }

  if(0 != errno) {
    syslog(LOG_WARNING, "Error reading file/directory within %s", dir);
  }
  closedir(dirp);
}

// Adds a hooli_file entry into a linked list
void addHooliFile(int checksum, char* filepath, hooli_file** file, int count)
{
  if(0 == count) {
    // First entry in list
    (*file)->filepath = filepath;
    (*file)->checksum = (int)checksum;
    (*file)->next = NULL;
  } else {
    hooli_file* temp = malloc(sizeof(hooli_file));
    temp->filepath = filepath;
    temp->checksum = (int)checksum;
    temp->next = NULL;
    (*file)->next = temp;
    *file = temp;
  }
}

// Logs all user arguments via syslog in debug mode
void logArguments(char* username, char* password, char* hostname, char* port,
  char* hooli_dir, int verbose_flag, char* hftp_hostname, char* hftp_port) 
{
  syslog(LOG_DEBUG, "Username: %s", username);
  syslog(LOG_DEBUG, "Password: %s", password);
  syslog(LOG_DEBUG, "Hostname: %s", hostname);
  syslog(LOG_DEBUG, "Port: %s", port);
  syslog(LOG_DEBUG, "Hooli dir: %s", hooli_dir);
  syslog(LOG_DEBUG, "Verbose: %d", verbose_flag);
  syslog(LOG_DEBUG, "HFTP Hostname: %s", hftp_hostname);
  syslog(LOG_DEBUG, "HFTP Port: %s", hftp_port);
}

// Calculates the checksum of one file
int calcChecksum(char* filename) {
  FILE* fp;                      // File pointer
  const int max_buffer = 100000; // Maximum bytes to read at once
  if(NULL != (fp = fopen(filename, "rb"))) {
    char* buffer = calloc(max_buffer, sizeof(char)); // Buffer string
    uLong checksum = crc32(0L, Z_NULL, 0);           // Initialize checksum
    int bytes_read;                                  // Bytes read from file

    // Read chunks of bytes from the file and update the checksum
    do {
      bytes_read = fread(buffer, 1, max_buffer, fp);
      checksum = crc32(checksum, (const void*)buffer, bytes_read);
    } while(max_buffer == bytes_read);

    free(buffer);
    // If we haven't reached the end of the file, something went wrong
    if(!feof(fp)) {
      fclose(fp);
      syslog(LOG_WARNING, "Error reading file %s", filename);
      errno = 0; // Reset errno and return -1
      return -1;
    }
    fclose(fp);
    return (int)checksum;
  } else {
    syslog(LOG_WARNING, "Error reading file %s", filename);
    errno = 0; // Reset errno and return -1
    return -1;
  }
}