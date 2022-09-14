#pragma once

#include <string>
#include <util/result.hpp>
#include <vector>

namespace fur::platform::io {

enum class IOError { GenericError };

/// Empty result for handling IO errors
using IOResult = util::Result<util::Empty, IOError>;

/// Create a new file on the disk
/// @param filename filename of the new file
/// @param size size of the new file
IOResult touch(const std::string& filename, size_t size);

/// Write bytes to file
/// @param filename filename of the target file
/// @param bytes number of files to write
/// @param offset where to write the bytes in the file
IOResult write_bytes(const std::string& filename,
                     const std::vector<uint8_t>& bytes, size_t offset);

/// Create subfolders
IOResult create_subfolders(const std::string& subfolders);

}  // namespace fur::platform::io
