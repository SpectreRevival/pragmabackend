#pragma once
#include <filesystem>

class PersistenceUtilities {
  private:
    static std::filesystem::path GetAppDataPath();

  public:
    static std::filesystem::path GetSavePath();
};