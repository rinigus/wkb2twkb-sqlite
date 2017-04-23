#include <iostream>
#include <sstream>
#include <string>
#include <deque>
#include <stdint.h>

#include <sqlite3pp.h>

extern "C" {
#include "liblwgeom.h"
}

int main(int argc, char** argv)
{
  sqlite3pp::database db;
  std::string database_path = "test.sqlite";
  std::string table = "osm_buildings";
  std::string column = "geometry";
  std::string precision_option = "7";
  int precision = 7;

  if ( argc < 4 || argc > 5 )
    {
      std::cout << "Please provide <database_filename> <table_name> <geometry_column_name> [<precision>]\n";
      std::cout << "If not provided, precision is 7, corresponding to 7 decimals after .\n";
      return -1;
    }
  
  database_path = argv[1];
  table = argv[2];
  column = argv[3];
  if (argc > 4) precision_option = argv[4];

  if (precision_option == "0") precision = 0;
  else if (precision_option == "1") precision = 1;
  else if (precision_option == "2") precision = 2;
  else if (precision_option == "3") precision = 3;
  else if (precision_option == "4") precision = 4;
  else if (precision_option == "5") precision = 5;
  else if (precision_option == "6") precision = 6;
  else if (precision_option == "7") precision = 7;
  else
    {
      std::cerr << "Unsupported precision: " << precision_option << "\n";
      std::cerr << "Please specify precision as a single digit from 0 to 7, inclusive\n";
      return -1;
    }

  if ( db.connect(database_path.c_str(),
                    SQLITE_OPEN_READWRITE) != SQLITE_OK )
    {
      std::cerr << "Error opening SQLite database: " << database_path << "\n";
      return -1;
    }

  db.execute( "PRAGMA journal_mode = OFF" );
  db.execute( "PRAGMA synchronous = OFF" );
  db.execute( "PRAGMA cache_size = 2000000" );
  db.execute( "PRAGMA temp_store = 2" );
  db.execute( "BEGIN TRANSACTION" );

  std::deque<int> ids;
  try 
    {
      sqlite3pp::query qry(db, ("SELECT rowid FROM " + table).c_str());
      
      for (auto v: qry)
	{
	  int n;
	  v.getter() >> n;
          ids.push_back(n);
	}
    }
   catch (sqlite3pp::database_error e)
    {
      std::cerr << "Exception while getting the list of ROWIDs: " << e.what() << std::endl;
      return -2;
    }

  std::cout << "Found records: " << ids.size() << "\n";

  size_t done = 0;
  size_t sz_wkb = 0;
  size_t sz_twkb = 0;
  for (int id: ids)
    {
      LWGEOM *geom = nullptr;

      // get blob value
      {
        std::stringstream cmdtxt;
        cmdtxt << "SELECT " << column << " FROM " << table << " WHERE rowid=" << id;
        try {
          sqlite3pp::query qry(db, cmdtxt.str().c_str() );
          sqlite3pp::query::iterator i = qry.begin(); // we expect only one result
          int bytes = (*i).column_bytes(0);
          const void *blob = (*i).get<const void*>(0);
          geom = lwgeom_from_wkb( (const uint8_t*)blob, bytes, 0 );
          sz_wkb += bytes;
        }
        catch (sqlite3pp::database_error e)
          {
            std::cerr << "Exception while getting BLOB value: " << e.what() << std::endl;
            return -2;
          }

        if (geom == nullptr)
          {
            std::cerr << "Failed to convert WKB: rowid=" << id << "\n";
            return -1;
          }
      }

      // convert to wktb
      size_t twkb_size;
      uint8_t* twkb = lwgeom_to_twkb(geom, 0, precision, 0, 0, &twkb_size);
      if (twkb == NULL)
        {
            std::cerr << "Failed to convert to TWKB: rowid=" << id << "\n";
            return -1;
        }
      
      // write in twkb
      std::stringstream cmdtxt;
      cmdtxt << "UPDATE " << table << " SET " << column << " = ? WHERE rowid = " << id;      
      sqlite3pp::command cmd(db, cmdtxt.str().c_str());
      cmd.bind(1, twkb, twkb_size, sqlite3pp::copy);
      try {
        cmd.execute();
      }
      catch (sqlite3pp::database_error e)
        {
          std::cerr << "Exception while updating a BLOB: " << id << " " << e.what() << std::endl;
          return -3;
        }

      lwfree(geom);
      free(twkb);

      sz_twkb += twkb_size;
      
      done++;
      //std::cout << "Done: " << done*100.0 / ids.size() << "%\n";
    }
  
  db.execute( "END TRANSACTION" );

  std::cout << "WKB size:  " << sz_wkb / 1024./1024. << " MB\n"
            << "TWKB size: " << sz_twkb / 1024./1024. << " MB\n"
            << "REDUCTION: " << 100.0 - (sz_twkb*100.0) / sz_wkb << "%\n";
  return 0;
}
