#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include<algorithm>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <regex>
#if __has_include(<windows.h>)
#include <windows.h>
#define WIN32_LEAN_AND_MEAN
#endif
//include other 
#ifdef _WIN32
constexpr auto OPERATING_SYS = "Windows32-bit";		//CHECK THE OS
#elif  __APPLE__ || __MACH__
constexpr auto OPERATING_SYS = "Apple";
#elif __linux__
constexpr auto OPERATING_SYS = "linux";
#elif __FreeBSD__
constexpr auto OPERATING_SYS = "FreeBSD";
#elif __unix || __unix__
constexpr auto OPERATING_SYS = "unix";
#else
constexpr auto OPERATING_SYS = "other";
#endif
#define NDEBUG	//remove assertions for production version
typedef std::unordered_map<std::string, std::string> syntaxmap;
class gcode
{
public:
	typedef std::vector<std::string> vector;
private:
	static const std::unique_ptr<syntaxmap> refsyntaxmapOI;					//OI only instance for syntaxmap
	vector gcode_;															//holds the code 
	static syntaxmap syntaxmapOI;										//OI of syntaxmap
	std::string fname;														//stores the file name instance 
	bool checkfile() noexcept(false);
	std::uint_fast8_t filestate;												//file bitmask state
	//TODO:algorithm 
	bool gcode_syntax();
	auto operator=(const gcode& g)->gcode& = delete;	//return type not dependent on args,(decltype deduce return type)
	gcode(const gcode& g)= delete;						//no copying 
public:													//deleted so not allowed 
	explicit gcode(const std::string filename = "default")noexcept(false);	
	void check();							
	bool display_fileexplorer();
	bool vectorfill()noexcept(false);
	void get() const
	{
		std::copy(gcode_.begin(), gcode_.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
	}
	~gcode() {}
	//copy constructor 
	//assignment operator (no except)
	//TODO: add functions to check for other errors, such as clearance 
};
   