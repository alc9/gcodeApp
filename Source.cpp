#include "gcode.h"
int main()
{
	gcode code{ "gcode.txt" }; //try Documents\gcode2.txt
	code.check();
	code.get();
	return 0;
}
//my gcode.txt test file looks like this
//G01 X1.0 Y5.0 !here
//G02 R2 Y1 !commentary
//G02 R2 Y1 !commentary
//R2 Y1 !commentary
