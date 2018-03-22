

#define buildstr(x)	#x
#define _source_pos(file, line) file ":" buildstr(line)
#define source_pos	 _source_pos(__FILE__, __LINE__)

