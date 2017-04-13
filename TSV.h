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
	struct Row
	{
		char*	lineStart;				// the real start of this line in the source buffer
		char**	columns;				// each column - will not but NULL but may be zero length
		int		columnCount;
		
		Row()
		: lineStart( NULL )
		, columns( NULL )
		, columnCount( 0 )
		{
		}
	};
	
	// source buffer is modified to add null terminators for each element
	char* 	source;
	Row*	rows;
	
	int 	sourceLength;
	int		rowCount;
	
	TSV()
	: source( NULL )
	, rows( NULL )
	, sourceLength( 0 )
	, rowCount ( 0 )
	{
	}
};

// clear down the buffers
static void TSVFree( TSV& tsv )
{
	for ( int i=0; i < tsv.rowCount; ++i )
		delete[] tsv.rows[ i ].columns;
	
	delete[] tsv.rows;
	delete[] tsv.source;
	tsv.rows = NULL;
	tsv.source = NULL;
	tsv.rowCount = 0;
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
			if ( tsv_out.rows != NULL )
			{
				// split the line with a NULL terminator
				*c++ = 0;
				
				// record the line start
				tsv_out.rows[ linecount ].lineStart = linestart;
			}
			
			linecount++;

			// find the next line - skip any blank ones
			while ( *c == '\n' || *c == '\r' || *c == 0 )
				c++;
			
			read = c;
		}
		
		if ( l == 0 )
		{
			tsv_out.rowCount = linecount;
			tsv_out.rows = new TSV::Row[ linecount ];
		}
	}
	
	// two passes per line:
	//  1. count the lines to allocate the line array
	//  1. split the lines on tabs
	for ( int l=0; l < tsv_out.rowCount; ++l )
	{
		TSV::Row& row = tsv_out.rows[ l ];
		
		// count the columns
		for ( int c=0; c < 2; ++c )
		{
			char* ll = row.lineStart;
			int column_count = 0;
			
			char* tab = strchr( ll, '\t' );
			while ( tab )
			{
				// second time round, store the columns
				if ( row.columns != NULL )
				{
					// insert null terminator to split the line into columns
					*tab = NULL;
					
					// record the column start
					row.columns[ column_count ] = ll;
				}
				
				column_count++;
				
				// find the next column or exit when we hit the line NLUL
				ll = tab+1;
				tab = strchr( ll, '\t' );
			}
			
			// remember the last one before line break
			if ( row.columns != NULL )
				row.columns[ column_count ] = ll;
			
			column_count++;
			
			// first time, allocate the columns
			if ( c == 0 )
			{
				row.columns = new char*[ column_count ];
				row.columnCount = column_count;
			}
		}
		
	}
	return true;
}

// 0 indexed cells, return NULL if out of bounds
static const char* TSVGetCell( const TSV& tsv, unsigned int x, unsigned int y )
{
	if ( y >= tsv.rowCount || x >= tsv.rows[ y ].columnCount )
		return NULL;
	
	const TSV::Row& row = tsv.rows[ y ];
	return row.columns[ x ];
}

// given a row 'y', find the column index that exactly matches the given one, write to x_out and return true
// return false if y is out of bounds
static bool TSVFindColumn( const TSV& tsv, unsigned int& x_out, unsigned int y, const char* column )
{
	if ( y >= tsv.rowCount )
		return false;

	const TSV::Row& row = tsv.rows[ y ];
	for ( unsigned int x=0; x < row.columnCount; ++x )
	{
		if ( strcmp( column, row.columns[ x ] ) == 0 )
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
	for ( int i=0; i < tsv.rowCount; ++i )
	{
		for ( int x=0; x < tsv.rows[ i ].columnCount; ++x )
			printf( "| %s ", tsv.rows[ i ].columns[ x ] );
		printf( "|\n" );
	}
}

#endif
