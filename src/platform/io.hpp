#pragma once

#include <filesystem>
#include <string>
#include <util/result.hpp>
#include <vector>

namespace fur::platform::io {

enum class IOError {
  GenericError,
  CannotOpenFile,
  InvalidPath,
  PathDoesNotExists,
  FileAlreadyExists,
  DirectoryAlreadyExists,
};

/// Empty result for handling IO errors
template <typename T>
using IOResult = util::Result<T, IOError>;
using util::Empty;

/// Create a new file on the disk
/// @param filename filename of the new file
/// @param size size of the new file
IOResult<Empty> touch(const std::string& filename, size_t size);

/// Check if a directory or file exists
/// @param filename path to check
IOResult<bool> exists(const std::string& filename);

/// Removes a file or directory
/// @param filename filename of the target file/directory
IOResult<Empty> remove(const std::string& filename);

/// Write bytes to file
/// @param filename filename of the target file
/// @param bytes number of files to write
/// @param offset where to write the bytes in the file
IOResult<Empty> write_bytes(const std::string& filename,
                            const std::vector<uint8_t>& bytes, size_t offset);

/// Create a nested folders structure
/// @param path path including all directories to create
/// @param skip_last skip last section of the path, used for files
/// @return constructed path or and error
IOResult<std::string> create_directories(const std::string& path,
                                         bool skip_last = false);

/// Load file content of a file
/// @param filepath filepath of the target file
/// @return loaded text or an error
IOResult<std::string> load_file_text(const std::string& filepath);

}  // namespace fur::platform::io
