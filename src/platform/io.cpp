#include <platform/io.hpp>

#include <fstream>
#include <sstream>

namespace fur::platform::io {
    
IOResult<Empty> touch(const std::string& filename, size_t size) {

    // Check if the file already exists
    auto existence = exists(filename);
    if (!existence.valid())
        return IOResult<Empty>::ERROR(IOError::GenericError);

    // If file already exists then something is wrong
    if (*existence)
        return IOResult<Empty>::ERROR(IOError::FileAlreadyExists);

    std::ofstream stream;
    stream.open(filename, std::ios::binary | std::ios::out);
    if (stream.good() && stream.is_open()) {

        // Go to the end of the file and add a 1
        stream.seekp(size - 1);
        stream << static_cast<unsigned char>(1);
        if (stream.good())
            return IOResult<Empty>::OK({});
    }

    return IOResult<Empty>::ERROR(IOError::GenericError);
}

IOResult<bool> exists(const std::string& filename) {
    std::error_code error;
    bool result = std::filesystem::exists(filename, error);
    if (error)
        return IOResult<bool>::ERROR(IOError::GenericError);
    return IOResult<bool>::OK(std::move(result));
}

IOResult<Empty> remove(const std::string& filename) {
    std::error_code error;
    std::filesystem::remove_all(filename, error);
    if (!error) 
        return IOResult<Empty>::OK({});

    return IOResult<Empty>::ERROR(IOError::GenericError);
}

IOResult<Empty> write_bytes(const std::string& filename, const std::vector<uint8_t>& bytes, size_t offset) {
    std::fstream stream;
    stream.open(filename, std::ios::binary | std::ios::in | std::ios::out);
    if (stream.good() && stream.is_open()) {

        // Go to desired location an write all bytes
        stream.seekp(offset, std::ios_base::beg);
        stream.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        if (stream.good())
            return IOResult<Empty>::OK({});
    }

    return IOResult<Empty>::ERROR(IOError::GenericError);
}

IOResult<std::string> create_directories(const std::string& path, bool skip_last) {
    
    std::string real_path;
    if (skip_last) {

        // Remove last part
        auto slash_tok = path.find_last_of('/');
        if (slash_tok == std::string::npos)
            return IOResult<std::string>::ERROR(IOError::InvalidPath);
            
        real_path = path.substr(0, slash_tok);
    }
    else {
        real_path = path;
    }

    std::error_code error;
    std::filesystem::create_directories(real_path, error);
    if (error) {
        if (error.default_error_condition().value() == static_cast<int>(std::errc::file_exists)) // EEXIST
            return IOResult<std::string>::ERROR(IOError::DirectoryAlreadyExists);
        return IOResult<std::string>::ERROR(IOError::GenericError);
    }

    return IOResult<std::string>::OK(std::move(real_path)); 
}

IOResult<std::string> load_file_text(const std::string& filepath) {
    std::ifstream file(filepath);
    if (file.good()) {

        std::ostringstream ss;
        ss << file.rdbuf();

        return IOResult<std::string>::OK(ss.str());
    }
    return IOResult<std::string>::ERROR(IOError::CannotOpenFile);
}

} // namespace fur::platform::io
