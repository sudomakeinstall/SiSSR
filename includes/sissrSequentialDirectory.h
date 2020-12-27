// STD
#include <filesystem>
#include <string>

// ITK
#include <itkMacro.h>

namespace sissr {

  struct SequentialDirectory {

    SequentialDirectory(const std::string& dir, const std::string& ext, const size_t N) :
      Dir(dir),
      Ext(ext),
      NumberOfFiles(N) {
        std::filesystem::create_directories(this->Dir);
      }

    SequentialDirectory(const std::string& dir, const std::string& ext) :
      Dir(dir),
      Ext(ext),
      NumberOfFiles(DetermineNumberOfFiles()) {}

    std::string PathForFrame(const size_t f) const {
      return this->Dir + std::to_string(f) + this->Ext;
    }
 
    bool DataExists() const {
      for (size_t f = 0; f < this->NumberOfFiles; ++f) {
        if (!std::filesystem::exists(this->PathForFrame(f))) {
          return false;
        }
      }
      return true;
    }

    const std::string Dir;
    const std::string Ext;
    const size_t NumberOfFiles;

    private:

    size_t DetermineNumberOfFiles() const {
      size_t N = 0;
      namespace fs = std::filesystem;
      while (fs::exists(this->PathForFrame(N))) {
        ++N;
      }
      itkAssertOrThrowMacro(N > 0, "At least one image must be supplied.");
      return N;
    }

  };

} // namespace sissr
