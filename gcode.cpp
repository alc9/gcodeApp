#include "gcode.h"

gcode::gcode(const std::string filename)noexcept(false)	//put into safe state
	: fname(filename), gcode_({ })
{
	filestate=0;
	if (!refsyntaxmapOI) { std::cerr << "points to null"; throw; }
	if (!(*refsyntaxmapOI == syntaxmapOI)) { std::cerr << "doesn't point to map"; throw; }
}
bool gcode::checkfile()	noexcept(false)	//check the file 
{								
	filestate = {0b0000'0000};							//filestatefalse till pass
	namespace fs = std::filesystem;			
	fs::path pathtofname=fname;		//via assignment due to later recursive call 
	auto permissions_file = [](fs::perms p)->bool
	{
		return ((p & fs::perms::owner_read) != fs::perms::none ? true : false);	//basic read permission
	};
	try							
	{
		if (fs::exists(pathtofname))	//exclusive 111^000
		{
			filestate = {0b0000'0001};
			if (permissions_file(fs::status(pathtofname).permissions()))	//test permissions
			{
				filestate = { 0b0000'0010 };
				if (fs::path(pathtofname).extension() == ".txt")
				{
					filestate = {0b0000'0100};
					return true;
				}
				else 
				{
					std::cout << "file " << fname << " must be a text file (.txt) try again,q to quit or e"
						<< " to select via file explorer" << std::endl;
					pathtofname.clear();
					std::getline(std::cin, fname);
					if (fname == "Q" | fname == "q") { std::cout << "program terminated"; exit(1); }
					else if (fname == "e") { if (!display_fileexplorer()) { std::cerr << "unrecognised os"; exit(1); } }
					else
						checkfile();
				}

			}
			else
			{
				std::cout << "file " << fname << " does not permit read operations try again,q to quit or e"
					<< "to select via file explorer" << std::endl;
				pathtofname.clear();
				std::getline(std::cin, fname);
				if (fname == "Q" | fname == "q") { std::cout << "program terminated"; exit(1); }
				else if (fname == "e") { if (!display_fileexplorer()) { std::cerr << "unrecognised os"; exit(1); } checkfile(); }
				else
					checkfile();
			}
		}
		else{
			std::cout << "file " << fname << " does not exist try again,q to quit or e to select via"
				<< " file explorer"<< std::endl;
			pathtofname.clear();
			std::getline(std::cin, fname);
			if (fname == "Q" | fname == "q") { std::cout << "program terminated"; exit(1); }
			else if (fname == "e") { if (!display_fileexplorer()) { std::cerr << "unrecognised os"; exit(1); } checkfile(); }
			else	
				checkfile();
			}
	}
	catch (const std::exception& e) { std::cerr << "ERROR " << e.what()<<std::endl; return false; }

	
}

bool gcode::display_fileexplorer() 
{	//TODO: extend file_explorer for other OS
	 
	assert(fname == "e");	//filename must be e for function to be called 
	if (OPERATING_SYS == "Windows32-bit")
	{
		fname = []()->std::string 
		{

			const int BUFSIZE = 1024;
			char buffer[BUFSIZE] = { 0 };
			OPENFILENAMEA ofns = { 0 };
			ofns.lStructSize = sizeof(ofns);
			ofns.lpstrFile = buffer;
			ofns.nMaxFile = BUFSIZE;
			ofns.lpstrTitle = "Choose a text file";
			GetOpenFileNameA(&ofns);
			return buffer;
		}();
		return true;
	}
	
	else if (OPERATING_SYS == "other")
		{
			return false;
		}
	
		else			
		{	
		auto anded = (filestate & 7);
		switch (anded)
		{
			case(0)
				:std::cerr << "file does not exist";
			case(1)
				:std::cerr << "file exists but doesn't have read permissions";
			case(2)
				:std::cerr << "file has the wrong extension";
			case(4)
				:std::cerr << "file is in the correct state";
			default
				:std::cerr << "file is in undefined state";
		}
		exit(1);	
		}
}	
	

bool gcode::vectorfill()noexcept(false)
{	//allow option for file explorer
	
		std::ifstream inf(fname, std::ios_base::in);	//open for reading 
		assert(inf.is_open());	//if assertion then file_check failed to validate
			try
			{
				std::string line;
				while (std::getline(inf, line))
				{
					std::string temp{};
					std::copy_if(line.begin(), line.end(), std::back_inserter(temp), [commentary = false, line](const char& ch)mutable
					{
						switch (ch)
						{
							case '!'
							:commentary = true; return false;
								default
							:return(commentary == true ? false : true);
								throw;
						}
					}
					);
					gcode_.emplace_back(temp);
					line.clear();
					temp.clear();
				}
			}
			catch (const std::exception& e)		//from derived down to base class 
			{
				std::cout << e.what();
			}
			return true;
		
		
}
void gcode::check()		
{
	
	if (checkfile())
	{
		if (vectorfill())
		{
			if (gcode_syntax())
			{
				return;
			}
			else std::cerr << "gcode syntax not checked" << std::endl; return;
		}

		else std::cerr << "error filling vector"; return;
	}
	else std::cerr << "error checking file"; return;
}
//template?
bool gcode::gcode_syntax()
{
	std::string regexform;
	int elements = syntaxmapOI.size();
	std::for_each(gcode_.begin(), gcode_.end(), [&, state = std::string{ "na" }, passedfirstline = false](std::string& str)mutable
	{
		int pos{ 0 };
		bool atend{ false };	//uninitialized
		regexform = str;
		bool found{ false };
		for (auto& it : syntaxmapOI)
		{
			pos++;
			auto val = str.find(std::string{ it.first });
			if (!(val == std::string::npos))	//if true set the state  
			{
				passedfirstline = true;
				state = std::string{ it.first };
				//bool whether syntax is correct,via if 
				auto checkingsyntax = [&]()->bool
				{	//in correct form for map
					regexform.erase(remove_if(regexform.begin(), regexform.end(), [](char c) { return !isalpha(c); }), regexform.end());
					remove_if(regexform.begin(), regexform.end(), isspace);
					std::string syntaxinstance = std::string{ it.second };	//error reading characters of string

					std::regex check(syntaxinstance);
					return ((!std::regex_match(regexform, check)) ? true : false);
				};
				if (!checkingsyntax())
				{
					//insert error comment to end of str
					std::string sub = " !error syntax must match for: " + state;
					str = str + sub;
					found == true;
					break;
				}
				else { found == true; break; }
			}
			if (pos == elements) { atend = true; }

		}
		if ((passedfirstline == true) && (atend == true) && (found == false))	//passedfirstline, at the end, and hasn't already found it 
		{//compare state syntax with line
			auto checking = [&]()->bool
			{	//in correct form for the map
				regexform.erase(remove_if(regexform.begin(), regexform.end(), [](char c) { return !isalpha(c); }), regexform.end());
				remove_if(regexform.begin(), regexform.end(), isspace);//compare it with regexform
				std::string syntaxinstance = syntaxmapOI[state];
				std::regex check(syntaxinstance);
				return ((!std::regex_match(regexform, check)) ? true : false);
			};
			if (!checking())
			{
				std::string sub = " ! error syntax must match for: " + state;
				str = str + sub;
				passedfirstline = true;
				atend = false;
			}
			atend = false;
		}
		else if (passedfirstline == false)
		{
			std::cerr << "First line is incorrect: first line identifier =" << passedfirstline << std::endl; return false;	//first line error
		}
	});
	return true;
}
///////////////////////initialize statics////////////////////////////
//https://machmotion.com/documentation/Software/Mach4/Mach4-G-and-M-Code-Reference-Manual.pdf
 syntaxmap gcode::syntaxmapOI = []()->std::unordered_map<std::string, std::string>		//n.b decltype if returning a const 
{
	std::unordered_map<std::string, std::string> syntax;
	syntax["G01"] = "^[N][G]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[F]?$";
	syntax["G02"] = "^[N][G]([X][Y]([Z][R]|[R]|[Z][I][J]|[I][J])|[X][Z]([Y][R]|[R]|[Y][I][K]|[I][K])|[Y][Z]([X][R]|[R]|[X][J][K]|[J][K]))[F]?$";
	syntax["G03"] = "^[N][G]([X][Y]([Z][R]|[R]|[Z][I][J]|[I][J])|[X][Z]([Y][R]|[R]|[Y][I][K]|[I][K])|[Y][Z]([X][R]|[R]|[X][J][K]|[J][K]))[F]?$";//G03
	syntax["G17"] = "^[N][G][G]?([X][Y]([Z][R]|[R]|[Z][I][J]|[I][J]))[F]?$";//G17
	syntax["G18"] = "^[N][G][G]?([X][Z]([Y][R]|[R]|[Y][I][K]|[I][K]))[F]?$";//G18
	syntax["G19"] = "^[N][G][G]?([Y][Z]([X][R]|[R]|[X][J][K]|[J][K]))[F]?$";//G19
	syntax["G04"] = "^[N][G][P]$"; //G04
	syntax["G09"] = "^[N][G]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[F]?";//G09
	syntax["G10"] = "^[N][G]{1,2}[L][P]([X]?([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])([A]?([B]?[C]?|[C]?)|[B][C]?|[C])[F]?$";//G10
	syntax["G12"] = "^[N][G]{1,2}([I][J][P]|[I][J]|[I]|[J]|[J][P]|[I][P])[F]?$";//G12
	syntax["G13"] = "^[N][G]{1,2}([I][J][P]|[I][J]|[I]|[J]|[J][P]|[I][P])[F]?$";//G13
	syntax["G16"] = "^[N][G]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[F]?$"; //UPDATE G16
	syntax["G17"] = "ENTER ME"; //polar coordinate G17 these would be added as the program is developed further 
	syntax["G18"] = "ENTER ME"; //polar coordinate G18
	syntax["G19"] = "ENTER ME"; //polar coordinate G19
	syntax["G00"] = "^[N][G]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[F]?$"; // G00
	syntax["GL"] = "^[N][G][G][G][G][G][G][G]$"; //g line, formatting
	syntax["GT"] = "^[N][T](.*)$"; //tool to add 
	syntax["AS"] = "^[N][M]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[F]?$"; //removing 
	syntax["AT"] = "^[N][E][N][D](.*)$";
	syntax["AU"] = "^[N][G]([X]([Y]?[Z]?|[Z]?)|[Y][Z]?|[Z])[D][E][S][M]$";


	return syntax;//assign memory 
}();	//not assigned therefore must call explicitly

const std::unique_ptr<syntaxmap> gcode::refsyntaxmapOI= std::make_unique<syntaxmap>(gcode::syntaxmapOI);

//TODO: add to syntaxmapOI 
