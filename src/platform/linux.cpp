#include <platform/io.hpp>

#include <sstream>

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

namespace fur::platform::io {

IOResult<Empty> touch(const std::string& filename, size_t size) {
  // Create new file
  int fd = creat(filename.c_str(), S_IRUSR | S_IWUSR);
  if (fd < 0) goto error;

  // Set correct size
  if (ftruncate64(fd, size) < 0) goto error;

  close(fd);
  return IOResult<Empty>::OK({});

error:
  close(fd);
  return IOResult<Empty>::ERROR(IOError::GenericError);
}

IOResult<Empty> write_bytes(const std::string& filename,
                     const std::vector<uint8_t>& bytes, size_t offset) {
  int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
  if (fd < 0) goto error;

  if (pwrite(fd, &bytes[0], bytes.size(), offset) < 0) goto error;

  close(fd);
  return IOResult<Empty>::OK({});

error:
  close(fd);
  return IOResult<Empty>::ERROR(IOError::GenericError);
}

IOResult<Empty> transfer_bytes(const std::string& source, const std::string& dest, size_t offset, size_t bytes) {

  auto sfd = open(source.c_str(), O_RDONLY);
  auto dfd = open(dest.c_str(), O_WRONLY);

  if (sfd == -1 || dfd == -1) {
    // TODO
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }

  /*
  struct stat sdf_stat;
  if (fstat(sfd, &sdf_stat) == -1) {
    // TODO
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }

  struct stat dfd_stat;
  if (fstat(dfd, &dfd_stat) == -1) {
    // TODO
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }
  */


  off_t _offset = static_cast<off_t>(offset);
  auto written = sendfile(dfd, sfd, &_offset, bytes);
  if (written == -1) {
    // TODO
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }

  return IOResult<Empty>::OK({});
}

IOResult<std::string> create_subfolders(const std::string& base, const std::vector<std::string>& subfolders) {
  std::stringstream result(base);
  for(auto& subfolder : subfolders) {
    result << subfolder << "/";

    // Try to create subfolder
    if (mkdir(result.str().c_str(), 0666) == -1) {
      // TODO
      return IOResult<std::string>::ERROR(IOError::GenericError);
    }
  }
  return IOResult<std::string>::OK(result.str());
}

}  // namespace fur::platform::io
