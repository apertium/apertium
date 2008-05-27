// This should really be defined elsewhere
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char *) buf, 1, max_size, yyin )) < 0 ) \
		YY_FATAL_ERROR( "input in flex scanner failed" );

#define fileno _fileno

#define isatty(x) 0

#define unlink _unlink
