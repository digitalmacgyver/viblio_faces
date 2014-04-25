/*
	Filesystem related functions based mostly on boost's cross platorm libraries for file system related functionality

	Date: 20/6/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#pragma once

#include <map>
#include <string>
namespace FileSystem
{
	bool GetDirectoryContents(const std::string &directory, std::map<std::string,bool> &contents);

	bool FileExists(const std::string &filename);

	bool DirectoryExists(const std::string &path);

	bool CreateDirectory(const std::string &dir_name);

	std::string FileSuffix(const std::string &filename);

	std::string FileNameMinusPath(const std::string &path);

	inline bool HasSuffix(const std::string &s, const std::string &suffix)
	{
		size_t foundPos = s.rfind(suffix);
		return (    suffix != "" &&
			foundPos != std::string::npos &&
			foundPos == (s.size() - suffix.size()));
	}
}
