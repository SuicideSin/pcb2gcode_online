//File Utility Source
//	Created By:		Mike Moss
//	Modified On:	11/08/2013

//Definitions for "file_util.hpp"
#include "file_util.hpp"

//Directory Entry Header (POSIX)
#include <dirent.h>

//File Stream Header
#include <fstream>

//Requires POSIX File System, if on Windows running Visual Studios, use:
//	http://www.softagalleria.net/dirent.php

//Windows Defines
#if defined(_WIN32)&&!defined(__CYGWIN__)
	#include <windows.h>
	#define RM "del /q "
	#define RMD "rd /s /b "

//Unix Defines
#else
	#include <stdlib.h>
	#define RM "rm -f "
	#define RMD "rm -rf "
#endif

//List Directory Function (Lists files and folders in directory as strings in a vector)
std::vector<std::string> msl::list_directory(const std::string& path_name)
{
	//Files Vector
	std::vector<std::string> files;

	//Open Directory
	DIR* dp=opendir(path_name.c_str());

	//While Directory is Opened
	while(dp!=NULL)
	{
		//Open Node
		dirent* np=readdir(dp);

		//If No Nodes, Close Directory and Break
		if(np==NULL)
		{
			closedir(dp);
			break;
		}

		//Create String for Node Name
		std::string node_name(np->d_name);

		//Add Node to Vector
		if(node_name!="."&&node_name!="..")
			files.push_back(node_name);
	}

	//Return Vector
	return files;
}

//List Directory Files Function (Lists files in directory as strings in a vector)
std::vector<std::string> msl::list_directory_files(const std::string& path_name)
{
	//Files Vector
	std::vector<std::string> files;

	//Open Directory
	DIR* dp=opendir(path_name.c_str());

	//While Directory is Opened
	while(dp!=NULL)
	{
		//Open Node
		dirent* np=readdir(dp);

		//If No Nodes, Close Directory and Break
		if(np==NULL)
		{
			closedir(dp);
			break;
		}

		//Create String for Node Name
		std::string node_name(np->d_name);

		//Determine if Node is a File
		bool file=(np->d_type==DT_REG);

		//Add Node to Vector
		if(node_name!="."&&node_name!=".."&&file)
			files.push_back(node_name);
	}

	//Return Vector
	return files;
}

//List Directory Folders Function (Lists folders in directory as strings in a vector)
std::vector<std::string> msl::list_directory_folders(const std::string& path_name)
{
	//Files Vector
	std::vector<std::string> files;

	//Open Directory
	DIR* dp=opendir(path_name.c_str());

	//While Directory is Opened
	while(dp!=NULL)
	{
		//Open Node
		dirent* np=readdir(dp);

		//If No Nodes, Close Directory and Break
		if(np==NULL)
		{
			closedir(dp);
			break;
		}

		//Create String for Node Name
		std::string node_name(np->d_name);

		//Determine if Node is a Folder
		bool folder=(np->d_type==DT_DIR||np->d_type==DT_LNK);

		//Add Node to Vector
		if(node_name!="."&&node_name!=".."&&folder)
			files.push_back(node_name);
	}

	//Return Vector
	return files;
}

//List Directory JSON Function (Lists directories and files inside them recursively, returned as a JSON object)
msl::json msl::list_directory_json(const std::string& path_name)
{
	//Create File and Folder JSON Objects
	msl::json files_json;
	msl::json folders_json;

	//Open Directory
	DIR* dp=opendir(path_name.c_str());

	//While Directory is Opened
	while(dp!=NULL)
	{
		//Open Node
		dirent* np=readdir(dp);

		//If No Nodes, Close Directory and Break
		if(np==NULL)
		{
			closedir(dp);
			break;
		}

		//Create String for Node Name
		std::string node_name(np->d_name);

		//Determine if Node is a File or Folder
		bool file=(np->d_type==DT_REG);
		bool folder=(np->d_type==DT_DIR||np->d_type==DT_LNK);

		//Skip Parent and Self, Causes Infinite Recursion Otherwise...
		if(node_name!="."&&node_name!="..")
		{
			//File, Add to File JSON
			if(file)
				files_json.set(msl::to_string(files_json.size()),node_name);

			//Folder, Add to Folder JSON, Recursively
			else if(folder)
				folders_json.set(msl::to_string(folders_json.size()),
					list_directory_json(path_name+"/"+node_name));
		}
	}

	//Set Sizes of JSON Objects
	files_json.set("size",files_json.size());
	folders_json.set("size",folders_json.size());

	//Create a Return JSON Object and Pack Everything Into It
	msl::json return_json;
	return_json.set("path",path_name);
	return_json.set("files",files_json);
	return_json.set("folders",folders_json);

	//Return JSON Object
	return return_json;
}

//File to String Function (Loads a file from disk as a string, returns false on error)
bool msl::file_to_string(const std::string& filename,std::string& data,const bool binary)
{
	//Read In Variable
	char buffer;

	//Open Flag
	std::ios_base::openmode flags=std::ios_base::in;

	//Binary Flag
	if(binary)
		flags|=std::ios_base::binary;

	//Open File
	std::ifstream istr(filename.c_str(),flags);

	//Unset Skip White Space Flag
	istr.unsetf(std::ios_base::skipws);

	//Test Stream
	if(!istr)
		return false;

	//Clear Data
	data="";

	//Read File into Buffer
	while(istr>>buffer)
		data+=buffer;

	//Close File
	istr.close();

	//Return Good
	return true;
}

//String to File Function (Saves a string to disk as a file, returns false on error)
bool msl::string_to_file(const std::string& data,const std::string& filename,const bool binary)
{
	//Saved Variable
	bool saved=false;

	//Open Flag
	std::ios_base::openmode flags=std::ios_base::out;

	//Binary Flag
	if(binary)
		flags|=std::ios_base::binary;

	//Open File
	std::ofstream ostr(filename.c_str(),flags);

	//Write New File
	saved=(ostr<<data);

	//Close File
	ostr.close();

	//Return Saved
	return saved;
}

//Remove File Function (Removes a file from disk, returns false on error)
bool msl::remove_file(const std::string& filename)
{
	return (system((RM+filename).c_str())!=-1);
}

//Remove Directory Function (Removes a directory from disk recursively, returns false on error)
bool msl::remove_directory(const std::string& directory)
{
	return (system((RMD+directory).c_str())!=-1);
}

//File Exists Function (Checks if a file exists in path)
bool msl::file_exists(const std::string& filename,const std::string& path_name)
{
	std::vector<std::string> files=msl::list_directory(path_name);

	bool found=false;

	for(unsigned int ii=0;ii<files.size();++ii)
	{
		if(files[ii]==filename)
		{
			found=true;
			break;
		}
	}

	return found;
}