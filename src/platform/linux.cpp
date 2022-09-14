#include <platform/io.hpp>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>

namespace fur::platform::io {

IOResult touch(const std::string& filename, size_t size) {
  // Create new file
  int fd = creat(filename.c_str(), S_IRUSR | S_IWUSR);
  if (fd < 0) goto error;

  // Set correct size
  if (ftruncate64(fd, size) < 0) goto error;

  close(fd);
  return IOResult::OK({});

error:
  close(fd);
  return IOResult::ERROR(IOError::GenericError);
}

IOResult write_bytes(const std::string& filename,
                     const std::vector<uint8_t>& bytes, size_t offset) {
  int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
  if (fd < 0) goto error;

  if (pwrite(fd, &bytes[0], bytes.size(), offset) < 0) goto error;

  close(fd);
  return IOResult::OK({});

error:
  close(fd);
  return IOResult::ERROR(IOError::GenericError);
}

}  // namespace fur::platform::io
