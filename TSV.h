#ifndef TSV_HEADER_GUARD

#define TSV_HEADER_GUARD

// Tab Separated Values is like a whole list of ... tab separated values. It's really simple! Hooray!
//
// Usage:
//	1. Load a file
//  2. Call TSVParse
//  3. Call TSVGetCell a bunch of times to do your thing
//  4. Call TSVFree
//
struct TSV
{
	struct Line
	{
		char*	lineStart;			// the real start of this line in the source buffer
		char**	columns;				// each column - will not but NULL but may be zero length
		int		columnCount;
		
		Line()
		: lineStart( NULL )
		, columns( NULL )
		, columnCount( 0 )
		{
		}
	};
	
	// source buffer is modified to add null terminators for each element
	char* 	source;
	Line*	lines;
	
	int 	sourceLength;
	int		lineCount;
	
	TSV()
	: source( NULL )
	, lines( NULL )
	, sourceLength( 0 )
	, lineCount ( 0 )
	{
	}
};

// clear down the buffers
static void TSVFree( TSV& tsv )
{
	for ( int i=0; i < tsv.lineCount; ++i )
		delete[] tsv.lines[ i ].columns;
	
	delete[] tsv.lines;
	delete[] tsv.source;
	tsv.lines = NULL;
	tsv.source = NULL;
	tsv.lineCount = 0;
	tsv.sourceLength = 0;
}

// copy the source data, mutate and parse the copy
static bool TSVParse( TSV& tsv_out, const char* source )
{
	if ( !source )
		return false;
	
	const int len = strlen( source );
	if ( len == 0 )
		return false;

	// clone the buffer to mutate it
	tsv_out.sourceLength = len;
	tsv_out.source = new char[ len + 1 ];
	memcpy( tsv_out.source, source, len + 1 );

	// split into lines
	for ( int l=0; l < 2; ++l )
	{
		int linecount = 0;
		char* read = tsv_out.source;

		// skip leading blanks
		while ( read - tsv_out.source < tsv_out.sourceLength && ( *read == '\n' || *read == '\r' ) )
			read++;
		
		// count the lines
		while ( read - tsv_out.source < tsv_out.sourceLength )
		{
			char* linestart = read;
			char* c = linestart;

			// find the line break
			while ( *c != '\n' && *c != '\r' && *c != 0 )
				c++;

			// store the line start
			if ( tsv_out.lines != NULL )
			{
				// split the line with a NULL terminator
				*c++ = 0;
				
				// record the line start
				tsv_out.lines[ linecount ].lineStart = linestart;
			}
			
			linecount++;

			// find the next line - skip any blank ones
			while ( *c == '\n' || *c == '\r' || *c == 0 )
				c++;
			
			read = c;
		}
		
		if ( l == 0 )
		{
			tsv_out.lineCount = linecount;
			tsv_out.lines = new TSV::Line[ linecount ];
		}
	}
	
	// two passes per line:
	//  1. count the lines to allocate the line array
	//  1. split the lines on tabs
	for ( int l=0; l < tsv_out.lineCount; ++l )
	{
		TSV::Line& line = tsv_out.lines[ l ];
		
		// count the columns
		for ( int c=0; c < 2; ++c )
		{
			char* ll = line.lineStart;
			int column_count = 0;
			
			char* tab = strchr( ll, '\t' );
			while ( tab )
			{
				// second time round, store the columns
				if ( line.columns != NULL )
				{
					// insert null terminator to split the line into columns
					*tab = NULL;
					
					// record the column start
					line.columns[ column_count ] = ll;
				}
				
				column_count++;
				
				// find the next column or exit when we hit the line NLUL
				ll = tab+1;
				tab = strchr( ll, '\t' );
			}
			
			// remember the last one before line break
			if ( line.columns != NULL )
				line.columns[ column_count ] = ll;
			
			column_count++;
			
			// first time, allocate the columns
			if ( c == 0 )
			{
				line.columns = new char*[ column_count ];
				line.columnCount = column_count;
			}
		}
		
	}
	return true;
}

// 0 indexed cells, return NULL if out of bounds
static const char* TSVGetCell( const TSV& tsv, unsigned int x, unsigned int y )
{
	if ( y >= tsv.lineCount || x >= tsv.lines[ y ].columnCount )
		return NULL;
	
	const TSV::Line& line = tsv.lines[ y ];
	return line.columns[ x ];
}

// given a row 'y', find the column index that exactly matches the given one, write to x_out and return true
// return false if y is out of bounds
static bool TSVFindColumn( const TSV& tsv, unsigned int& x_out, unsigned int y, const char* column )
{
	if ( y >= tsv.lineCount )
		return false;

	const TSV::Line& line = tsv.lines[ y ];
	for ( unsigned int x=0; x < line.columnCount; ++x )
	{
		if ( strcmp( column, line.columns[ x ] ) == 0 )
		{
			x_out = x;
			return true;
		}
	}

	return false;
}

// print out the TSV columns separated by |
static void TSVPrint( const TSV& tsv )
{
	for ( int i=0; i < tsv.lineCount; ++i )
	{
		for ( int x=0; x < tsv.lines[ i ].columnCount; ++x )
			printf( "| %s ", tsv.lines[ i ].columns[ x ] );
		printf( "|\n" );
	}
}

#endif
