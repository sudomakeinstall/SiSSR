// STD
#include <filesystem>
#include <string>
#include <iostream>

// ITK
#include <itkMacro.h>

namespace sissr {

  struct SequentialDirectory {

    SequentialDirectory(const std::string& dir, const std::string& ext, const size_t N);
    SequentialDirectory(const std::string& dir, const std::string& ext);
    std::string PathForFrame(const size_t f) const;
    bool DataExists() const;

    const std::string Dir;
    const std::string Ext;
    const size_t NumberOfFiles;

    private:

    size_t DetermineNumberOfFiles() const;

  };

} // namespace sissr
