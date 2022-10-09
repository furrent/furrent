#include <platform/io.hpp>

#include <fstream>
#include <sstream>

namespace fur::platform::io {
    
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
