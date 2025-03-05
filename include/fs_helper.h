#ifndef FS_HELPER_H
#define FS_HELPER_H

#include <filesystem>
#include <string>
#include <iostream>

/**
 * @brief Filesystem helpers
*/
namespace FSHelper
{
	/**
	 * @brief 
	 * @param path Path to the file.
	 * If path contains filename it will be removed.
	 * @return bool True, if path created or already exists. 
	 * False otherwise.
	*/
	static bool CreateDir(const std::string& path)
	{
		std::filesystem::path dir(path);
		dir = std::filesystem::absolute(dir);
		dir = dir.remove_filename();
		if (std::filesystem::exists(dir)) return true;
		try
		{
			std::filesystem::create_directories(dir);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to create directory: " << dir.string() << " " << e.what() << std::endl;
			return false;
		}
		return true;
	};
};

#endif // !FS_HELPER_H
