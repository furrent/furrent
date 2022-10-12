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

  // If the file already exists then returns error
  if (access(filename.c_str(), F_OK) == 0)
    return IOResult<Empty>::ERROR(IOError::FileAlreadyExists);

  // Create new file and set correct size
  int fd = creat(filename.c_str(), 0777);
  if (fd < 0 || ftruncate64(fd, size) < 0) {
    close(fd);
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }

  close(fd);
  return IOResult<Empty>::OK({});
}

/// Removes a file or directory
IOResult<Empty> remove(const std::string& filename) {
  if (unlink(filename.c_str()) < 0) {

    // Target does not exists
    if (errno == ENOENT)
      return IOResult<Empty>::ERROR(IOError::PathDoesNotExists);

    // Try to remove directory
    if (errno == EISDIR && rmdir(filename.c_str()) < 0)
      return IOResult<Empty>::ERROR(IOError::GenericError);

    // Other error
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }
  return IOResult<Empty>::OK({});
}

IOResult<Empty> write_bytes(const std::string& filename,
                     const std::vector<uint8_t>& bytes, size_t offset) {
  int fd = open(filename.c_str(), O_CREAT | O_WRONLY, 0666);
  if (fd < 0 || pwrite(fd, &bytes[0], bytes.size(), offset) < 0) {
    close(fd);
    return IOResult<Empty>::ERROR(IOError::GenericError);
  }

  close(fd);
  return IOResult<Empty>::OK({});
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
  
  std::string real_base = (base.back() == '/' || base.size() == 0) ? base : base + '/';
  std::stringstream sstream; 
  sstream << base;

  for(size_t i = 0; i < subfolders.size(); i++) {
    sstream << subfolders[i];
    if (subfolders.size() >= 2 && i != subfolders.size() - 2)
      sstream << '/';

    // Try to create subfolder
    std::string folder = sstream.str();
    if (mkdir(folder.c_str(), 0777) == -1) {
      if (errno == EEXIST) {
        return IOResult<std::string>::ERROR(IOError::DirectoryAlreadyExists);
      }
      return IOResult<std::string>::ERROR(IOError::GenericError);
    }
  }

  return IOResult<std::string>::OK(sstream.str());
}

}  // namespace fur::platform::io