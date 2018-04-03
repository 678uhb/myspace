
#pragma once
#define __myspace_buildstr(x)	#x
#define __MYSPACE_SOURCE_POS(file, line) file ":" __myspace_buildstr(line)
#define MYSPACE_SOURCE_POS	 __MYSPACE_SOURCE_POS(__FILE__, __LINE__)

