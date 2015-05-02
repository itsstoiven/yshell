// This program was completed using pair programming.
// Steven Cruz (sicruz@ucsc.edu)
// Partner: Jason Ou (jaou@ucsc.edu)

// $Id: main.cpp,v 1.3 2014-06-11 13:52:31-07 - - $

// Libraries

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <unistd.h>

// Using

using namespace std;

// Header

#include "commands.h"
#include "debug.h"
#include "inode.h"
#include "util.h"

//
// scan_options
//    Options analysis:  The only option is -Dflags. 
//

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      // Checks for @ for flags
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) option << ": invalid option"
                       << endl;
            break;
      }
   }
   if (optind < argc) {
      complain() << "operands not permitted" << endl;
   }
}

//
// main -
//    Main program which loops reading commands until end of file.
//

int main (int argc, char** argv) {
   execname (argv[0]);
   cout << boolalpha; // Print false or true instead of 0 or 1.
   cerr << boolalpha;
   cout << argv[0] << " build " << __DATE__ << " " << __TIME__ << endl;
   scan_options (argc, argv);
   bool need_echo = want_echo();
   commands cmdmap;
   inode_state state;
   try {
      for (;;) {
         try {
            // Read a line, break at EOF, and echo print the prompt
            // if one is needed.
            cout << state.get_prompt() << " ";
            string line;
            getline (cin, line);
            if (cin.eof()) {
               if (need_echo) cout << "^D";
               cout << endl;
               DEBUGF ('y', "EOF");
               break;
            }
            if (need_echo) cout << line << endl;
			
			   // Checks for empty lines, if line is empty, ignore
            if (!line.compare("")) continue;
   
            // Split the line into words and lookup the appropriate
            // function.  Complain or call it.
            wordvec words = split (line, " \t");
            vector<string> wordVec = vector<string>(words.begin()+1, words.end());
			
			   // Checks for #, if it's a #, ignore
            if(words.at(0) == "#") continue;
			
            command_fn fn = cmdmap.at(words.at(0));
            fn (state, wordVec);
         } catch (yshell_exn& exn) {
            // If there is a problem discovered in any function, an
            // exn is thrown and printed here.
            complain() << exn.what() << endl;
         }
      }
   } catch (ysh_exit_exn& ) {
      // This catch intentionally left blank.
   }

   return exit_status_message();
}
