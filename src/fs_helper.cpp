#include "fs_helper.h"

/**
 * @brief Create directories from path.
 * @param path Path to the file/directory.
 * @param errMsg Error message.
 * If path contains filename it will be removed.
 * @return bool True, if path created or already exists.
 * False otherwise.
*/

bool FSHelper::CreateDir(const std::string& path, std::string& errMsg)
{
    std::filesystem::path dir(path);
    dir = std::filesystem::absolute(dir);
    dir = dir.remove_filename();
    if(std::filesystem::exists(dir))
        return true;
    try
    {
        // May return false, if path contains trailing separatror. So we handle exception on fault, not bool result.
        std::filesystem::create_directories(dir);
    }
    catch(const std::exception& e)
    {
        errMsg = e.what();
        return false;
    }
    return true;
}
