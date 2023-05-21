#include <iostream>
#include <fstream>
#include <chrono> //std::chrono
#include <ctime>  //ctime()
#include <string>
#include <future> //std::future
#include <mutex> //std::mutex
#pragma comment(lib, "Version.lib")

#include "color.hpp"
#include "CSV/CSV.hpp"
#include "Memory/Memory.h"

#define PATTERN_FILE "Patterns.txt"
#define DUMP_FILE "Offsets.hpp"

std::ofstream output;

enum InputFields
{
	FIELDS_TYPE,
	FIELDS_NAME,
	FIELDS_PATTERN,
	FIELDS_OFFSET,
	FIELDS_TYPESIZE
};
std::string GetFileVersionString(const char* fileName)
{
    DWORD dwHandle = -1;
    DWORD dwSize = GetFileVersionInfoSizeA(fileName, &dwHandle);
    if (dwSize == 0 || dwHandle == -1)
        return "";


    std::vector<BYTE> buffer(dwSize);
    if (!GetFileVersionInfoA(fileName, dwHandle, dwSize, &buffer[0]))
        return "";

    VS_FIXEDFILEINFO* pFileInfo;
    UINT uiLen;
    if (!VerQueryValueA(&buffer[0], "\\", (LPVOID*)&pFileInfo, &uiLen))
        return "";

    int major = HIWORD(pFileInfo->dwFileVersionMS);
    int minor = LOWORD(pFileInfo->dwFileVersionMS);
    int build = HIWORD(pFileInfo->dwFileVersionLS);
    int revision = LOWORD(pFileInfo->dwFileVersionLS);

    char versionStr[32];
    sprintf_s(versionStr, "%d.%d.%d.%d", major, minor, build, revision);

    return versionStr;
}
bool ParseFileToStruct(std::vector<std::shared_ptr<PatternStruct>> &vector)
{
	jay::util::CSVread csv_read(PATTERN_FILE,
		jay::util::CSVread::strict_mode
		| jay::util::CSVread::text_mode
		//jay::util::CSVread::process_empty_records
		//| jay::util::CSVread::skip_utf8_bom_check
	);

	if (csv_read.error)
	{
		std::cerr << PATTERN_FILE << " failed: " << csv_read.error_msg << std::endl;
		return false;
	}

	while (csv_read.ReadRecord())
	{
		auto ps = std::make_shared<PatternStruct>();

		for (unsigned i = 0; i < csv_read.fields.size(); ++i)
		{
			if (std::strcmp(csv_read.fields[FIELDS_TYPE].c_str(), "OFFSET") == 0)
				ps->type = TYPE_OFFSET;
			else if (std::strcmp(csv_read.fields[FIELDS_TYPE].c_str(), "ADDRESS") == 0)
				ps->type = TYPE_ADDRESS;
			else if (std::strcmp(csv_read.fields[FIELDS_TYPE].c_str(), "FUNCTION") == 0)
				ps->type = TYPE_ADDRESS_FUNCTION;
			else
				continue;

			if(csv_read.fields.size() > 4)
				ps->type_size = std::stoi(csv_read.fields[FIELDS_TYPESIZE]);
			else
				ps->type_size = 4; //4 is a uintptr_t

			ps->name = csv_read.fields[FIELDS_NAME];
			ps->pattern = csv_read.fields[FIELDS_PATTERN];
			ps->offset = std::stoi(csv_read.fields[FIELDS_OFFSET]);
		}

		if(ps->type != TYPE_INVALID)
			vector.push_back(ps);
	}

	if (csv_read.eof && (csv_read.record_num == csv_read.end_record_num))
	{
		return true;
	}

	return false;
}

void CreateDumpFile(const char*dumpfile)
{
	//Create file
	output.open(DUMP_FILE);

	//Get Time Now
	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	//Convert time to ctime format
	char str_time[MAXCHAR];
	ctime_s(str_time, MAXCHAR, &now);

	//write in file
	output << "#pragma once" << std::endl << std::endl;
	output << "/*" << std::endl;
	output << "Async offset dumper" << std::endl;
	output << "Improved version of (@Ph4nton lol-dumper) by LeftSpace" << std::endl;
	output << str_time;
	output << "*/" << std::endl << std::endl;
	output << "/*" << GetFileVersionString(dumpfile) << "*/"<< std::endl;
}



int main(int argc, const char* argv[])
{
	auto Memory = std::make_shared<CMemory>();
	std::vector<std::shared_ptr<PatternStruct>> pattern_struct;

	if (argc < 2) {
		std::cout << "Correct usage: " << argv[0] << " \"League of Legends.exe\"" << std::endl;
		system("pause");
		return 0;
	}

	if (!ParseFileToStruct(pattern_struct)) {
		system("pause");
		return 0;
	}

	if (!Memory->Initialize(argv[1])) {
		system("pause");
		return 0;
	}

	SetConsoleTitleA("Improved version of (@Ph4nton lol-dumper) by LeftSpace");
	//Create output file
	CreateDumpFile(argv[1]);

	struct TResult
	{
		uintptr_t address = 0;
		std::shared_ptr<PatternStruct> obj = nullptr;
	};

	auto pWork = [](std::shared_ptr<CMemory> mem,std::shared_ptr<PatternStruct> obj){
		auto address = mem->Pattern(obj);
		return TResult{address , obj};
	};
	std::vector<std::future<TResult>> cworkers;


	for (auto obj : pattern_struct)
	{
		cworkers.push_back(std::async(std::launch::async,pWork,Memory,obj));
	}


	for(auto const &work : cworkers)
		 work.wait();

	std::vector<TResult>results;
	for(auto &work : cworkers)
	{
		auto result = work.get();
		results.push_back(result);
	}

	std::sort(results.begin(),results.end(),[&](const TResult&a, const TResult&b){return a.address > b.address;});
	for(auto const&result : results)
	{
		std::cout << (result.address > 0 ? dye::white(result.obj->name) : dye::red(result.obj->name) )  <<  (result.address > 0 ? dye::green(": 0x") : dye::red(": 0x"))  << std::hex << std::uppercase << (result.address > 0 ? dye::green(result.address) : dye::red(result.address)) << std::endl;
		output << "#define " << result.obj->name <<  " 0x" << std::hex << std::uppercase << result.address << "\t//" << result.obj->pattern << std::endl;
	}

	const auto validOffsets = std::count_if(results.begin(),results.end(),[&](const TResult&res){ return res.address > 0;});
	const auto invalidOffsets = std::count_if(results.begin(),results.end(),[&](const TResult&res){ return res.address <= 0;});


	std::cout<< "valid: " << dye::green(validOffsets)  << " invalid: " << dye::red(invalidOffsets)<< std::endl;

	output.close();
	system("pause");

	return 0;
}