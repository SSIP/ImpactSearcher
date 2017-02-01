#include "filesystem.h"

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#else
	#include <dirent.h>
	#include <unistd.h>
#endif

list<fileInf> getFiles(tstring dir, tstring extension) {
	// note: all FAT file systems will return files unsorted, so please do not use FAT!

	list<fileInf> retval;

#ifdef WIN32

	// build search tstring (also accept trailing backslash in dir parameter)
	tstring directory = L"\\\\?\\" + dir + (dir[dir.length() - 1] != L'\\' ? L"\\" : L"");

	WIN32_FIND_DATA ffd;
	HANDLE hfind = nullptr;

	if ((hfind = FindFirstFile((directory + L"*").c_str(), &ffd)) == INVALID_HANDLE_VALUE)
		return retval; // no files found

	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// this is a subdir (or . or ..), ignore it
		}
		else {
			// this is a file, check its extension
			wchar_t *pExtension = ffd.cFileName + wcslen(ffd.cFileName) - extension.length();

			if (wcscmp(pExtension, extension.c_str()) == 0) {
				// save full path and size
				fileInf fi;
				fi.name = directory + ffd.cFileName;
				fi.size = (uint64_t)ffd.nFileSizeHigh << 32 | ffd.nFileSizeLow;
				retval.push_back(fi);
			}
		}
	} while (FindNextFile(hfind, &ffd));

	FindClose(hfind);

#else

	// build search tstring (also accept trailing slash in dir parameter)
	tstring directory = dir + (dir[dir.length() - 1] != L'/' ? L"/" : L"");

	DIR *dir;
	dirent *file;
	stat st;

	if ((dir = opendir(directory.c_str())) == nullptr)
		return retval;

	while ((file = readdir(dir)) != nullptr) {
		// check for . or .. directories
		if (file->d_name[0] == '.')
			continue; // this quick and dirty but should suffice

		// stat() the file
		tstring fileName = directory + file->d_name;
		if (stat(fileName.c_str(), &st) == -1)
			continue;

		// check for generic directories
		if ((st.st_mode & S_IFDIR) != 0)
			continue;

		// this is a file, check its extension
		wchar_t *pExtension = file->d_name + wcslen(file->d_name) - extension.length();

		if (wcscmp(pExtension, extension.c_str()) == 0) {
			// save full path and size
			fileInf fi;
			fi.name = fileName;
			fi.size = st.st_size;
			retval.push_back(fi);
		}
	}
	closedir(dir);

#endif

	return retval;
}

void deleteFiles(list<fileInf> files) {

#ifdef WIN32

	for (auto it = files.begin(); it != files.end(); ++it)
		DeleteFile(it->name.c_str());

#else

	for (auto it = files.begin(); it != files.end(); ++it)
		unlink(it->name.c_str());

#endif

}
