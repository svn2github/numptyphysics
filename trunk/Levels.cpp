
#include <sys/types.h>
#include <dirent.h>

#include "Levels.h"
#include "zipfile.h"

using namespace std;

static int rankFromPath( const string& p )
{
  const char *c = p.data();
  size_t i = p.rfind('/');
  if ( i != string::npos ) {
    c += i+1;
    if ( *c++=='L' ){
      int rank=0;
      while ( *c>='0' && *c<='9' ) {
	rank = rank*10 + (*c)-'0';
	c++;
      }
      return rank;
    }
  }
  return 9999;
}

Levels::Levels( int numFiles, const char** names )
{
  for ( int d=0;d<numFiles;d++ ) {
    addPath( names[d] );
  }
}

bool Levels::addPath( const char* path )
{
  int len = strlen( path );
  if ( strcasecmp( path+len-4, ".nph" )==0 ) {
    addLevel( path, rankFromPath(path) );
  } else {
    DIR *dir = opendir( path );
    if ( dir ) {
      struct dirent* entry;
      while ( (entry = readdir( dir )) != NULL ) {
	string full( path );
	full += "/";
	full += entry->d_name;
	int n = strlen( entry->d_name );
	if ( strcasecmp( entry->d_name+n-4, ".zip" )==0 ) {
	  scanCollection( full, rankFromPath(full) );
	} else if ( strcasecmp( entry->d_name+n-4, ".nph" )==0 ) {
	  addLevel( full, rankFromPath(full) );
	}
      }
      closedir( dir );
    } else {
      printf("bogus level path %s\n",path);
    }
  }
  return true;
}

bool Levels::addLevel( const string& file, int rank )
{
  printf("found level %s\n",file.c_str());
  LevelDesc *e = new LevelDesc( file, rank );
  for ( int i=0; i<m_levels.size(); i++ ) {
    if ( m_levels[i]->rank > rank ) {
      printf("addLevel %s at %d\n",file.c_str(),i);
      m_levels.insert(i,e);
      return true;
    }
  }
  m_levels.append( e );
  return true;
}


bool Levels::scanCollection( string& file, int rank )
{
  printf("found collection %s\n",file.c_str());
#if 1
  
#else
  HZIP hz = OpenZip( (void*)file.c_str(), 0, ZIP_FILENAME );
  if ( hz ) {
    int index = 0;
    ZIPENTRY entry;
    while ( GetZipItem( hz, index, &entry )==ZR_OK ) {
      addLevel( file, rank, index );
      index++;
    }
    CloseZip( hz );
  }
#endif
  return false;
}

int Levels::numLevels()
{
  return m_levels.size();
}

const std::string& Levels::levelFile( int i )
{
  if ( i < m_levels.size() ) {
    return m_levels[i]->file;
  }
  throw "invalid level index";
}


int Levels::levelSize( int l )
{
  if ( m_levels[l]->index >= 0 ) {
  } else {
  }
  return 0;
}

bool Levels::load( int l, void* buf, int buflen )
{
  if ( m_levels[l]->index >= 0 ) {
  } else {
  }
  return true;
}

