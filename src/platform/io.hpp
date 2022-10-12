#pragma once

#include <string>
#include <util/result.hpp>
#include <vector>

namespace fur::platform::io {

enum class IOError { GenericError, CannotOpenFile };

/// Empty result for handling IO errors
template<typename T>
using IOResult = util::Result<T, IOError>;
using util::Empty;

/// Create a new file on the disk
/// @param filename filename of the new file
/// @param size size of the new file
IOResult<Empty> touch(const std::string& filename, size_t size);

/// Write bytes to file
/// @param filename filename of the target file
/// @param bytes number of files to write
/// @param offset where to write the bytes in the file
IOResult<Empty> write_bytes(const std::string& filename,
                     const std::vector<uint8_t>& bytes, size_t offset);

/// Read bytes from source and copy them to destination
/// @param source filepath of the source file
/// @param dest filepath of the destination file
/// @param offset offset from the beginning of the source file
/// @param bytes how many bytes to copy
IOResult<Empty> transfer_bytes(const std::string& source, const std::string& dest, size_t offset, size_t bytes);

/// Create a nested folders structure
/// @param base base of the nested structure
/// @param subfolders sequence of nested folders
/// @return constructed path or and error 
IOResult<std::string> create_subfolders(const std::string& base, const std::vector<std::string>& subfolders);

/// Load file content of a file
/// @param filepath filepath of the target file
/// @return loaded text or an error
IOResult<std::string> load_file_text(const std::string& filepath);

}  // namespace fur::platform::io
