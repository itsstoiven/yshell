// This program was completed using pair programming.
// Steven Cruz (sicruz@ucsc.edu)
// Partner: Jason Ou (jaou@ucsc.edu)

// $Id: commands.cpp,v 1.11 2014-06-11 13:49:31-07 - - $

// Header Files

#include "commands.h"
#include "debug.h"

// Command Map

commands::commands(): map ( {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
   {"rmr"   , fn_rmr   }
}) {}

command_fn commands::at (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   command_map::const_iterator result = map.find (cmd);
   if (result == map.end()) {
      throw yshell_exn (cmd + ": no such function");
   }
   return result->second;
}

// Command Implementations

// cat pathname...
// The contents of each file is copied to stdout. An error is reported
//    if no files are specified, a file does not exist, or is a directory

void fn_cat (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() > 0) {
      for (auto word = words.begin(); word != words.end(); ++word) {
         bool check = ((*word)[0] == '/');
         cout << state.readfile( split(*word, "/") , check) << endl;
      }
   } else {
      throw yshell_exn("cat:: no file given");
   }
}

// cd [pathname]
// The current directory is set the pathname given. If no pathname is specified,
//    the root directory (/) is used. It is an error if the pathname does not exist
//    or is a plain file, or if more than one operand is given

void fn_cd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() > 1) { cout << "Usage: cd [pathname]" << endl; return; }

   auto dir = state.get_contents();
   if (words.size() > 0) {
      bool check = false;
      if (words[0][0] == '/') {check = true;}
      wordvec dirName = words;
      dirName[0] = dirName[0] + "/";
      wordvec path = dirName;
      try {
         state.set_cwd(path, check);
      } catch (yshell_exn) {
         throw yshell_exn("cd :: no such directory");
      }
   } else {
      state.set_cwd(words, true);
   }
}

// echo [words... ]
// The string, which may be empty, is echoed to stdout on a line by itself

void fn_echo (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   cout << words << endl;
}

// exit [status]
// Exit the program with the given status. If the status is missing, exit
//    with status 0. If a non-numeric argument is given, exit with status 127

void fn_exit (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   int x;
   if (words.size() == 1) {
      try {
         x = stoi(words.at(0));
         exit_status::set(x);
      } catch (std::invalid_argument& e) {
         throw yshell_exn("exit: " + words.at(0)
                          + ": exit status must be numeric");
      }
   } else if (words.size() > 1) {
      throw yshell_exn("exit: usage: exit [status]");
   }

   throw ysh_exit_exn();
}

// ls [pathname... ]
// A description of the files or directories are printed to stdout.
//    It is an error if any of the file or directories does not exist.
//    If no pathname is specified, the current working directory is used.
//    If a pathname specified is a directory, then the contents of the directory are listed.
//    A directory listed within a directory is shown by a terminating slash.
//    Elements of a directory are listed lexicographically.

void fn_ls (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   bool check = false;
   string orig = pathname(state.get_contents().at("."), state.get_root()->get_inode_nr());
   if (words.size() > 0 ) {
      for (auto word = words.begin(); word != words.end(); ++word) {
         if ((*word)[0] == '/') {check = true;}
         state.set_cwd(split(*word, "/"), check);
         print_directory(state);
         state.set_cwd(split(orig, "/"), true);
      }
   } else {
      print_directory(state);
   }
}

// lsr [pathname... ]
// As for ls, but a recursive depth-first preorder traversal is done
//    for subdirectories

void fn_lsr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

// make pathname [words... ]
// The file specified is created and the rest of the words are put in that file.
//   If the file already exists, a new one is not created, but its contents are replaced.
//    It is an error to specify a directory. If there are no words, the file is empty

void fn_make (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   bool check = false;
   wordvec data ;
   if (words[0][0] == '/') {check = true;}
   wordvec pathname = split(words[0], "/") ;
   for (auto inc = words.begin() + 1; inc != words.end(); ++inc) {
      data.push_back(*inc);
   }
   state.add_file(data, pathname, check);
}

// mkdir pathname
// A new directory is created. It is an error if a file or directory of the same
//    name already exists, or if the complete pathname to the parent of this
//    new directory does not already exist. Two entries are added to the directory,
//    namely dot (.) and dotdot (..). Directory entries are always kept in
//    sorted lexicographic order.

void fn_mkdir (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   bool check = false;
   for (auto word = words.begin(); word != words.end(); ++word) {
      if (words[0][0] == '/') {check = true;}
      wordvec dirName = words;
      dirName[0] = dirName[0] + "/";
      wordvec path = dirName;
      try {
         state.add_directory(path, check);
      } catch (yshell_exn e) {
         string except = e.what();
         throw yshell_exn("mkdir::" + except);
      }
   }
}
// prompt [string]
// Set the prompt to the words specified on the command line. Each word is
//    separated from the next by one space and the prompt itself is terminated
//    by an extra space. The default prompt is a single percent sign and a
//    space (% ).

void fn_prompt (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() == 0) { cout<< "Usage: prompt [string]" << endl; return; }

   string str {};
   str = str + words[0] + " ";
   state.set_prompt(str);
}

// pwd
// Prints the current working directory.

void fn_pwd (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   string path = pathname(state.get_contents().at("."), state.get_root()->get_inode_nr());
   if (path.size() == 1) {
      cout << path << endl;
   } else {
      cout << path.substr(0, path.size()-1) << endl;
   }
}

// rm pathname
// The specified file or directory is deleted (removed from its parent’s list of
//    files and subdirectories). It is an error for the pathname not to exist. If
//    the pathname is a directory, it must be empty.

void fn_rm (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if (words.size() == 0) { cout<< "Usage: rm [pathname]" << endl; return; }

   //cannot be . and .. or a directory
   for (auto word = words.begin(); word != words.end(); ++word) {
      bool check = false;
      if ((*word)[0] == '/') {check = true;}
      wordvec pathname = split(*word, "/") ;
      try {
         state.remove(pathname, check, true);
      } catch (yshell_exn e) {
         string except = e.what();
         throw yshell_exn("rm::" + except );
      }
   }
}

// rmr pathname
// A recursive removal is done, using a depth-first postorder traversal.

void fn_rmr (inode_state& state, const wordvec& words) {
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

// exit status message

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

// returns the pathname

string pathname(inode_ptr ip, int root_nr) {
   int my_inode_nr = ip->get_inode_nr();
   vector<string> pathname;
   while (my_inode_nr != root_nr) {
      ip = ip->get_contents().at("..");
      for (auto& content : ip->get_contents()) {
         if (content.second->get_inode_nr() == my_inode_nr) {
            pathname.insert(pathname.begin(), content.first);
         }
      }
      my_inode_nr = ip->get_inode_nr();

   }
   string pathstring = "/";
   for (auto& str : pathname) {
      pathstring = pathstring  + str;
   }
   return pathstring;
}

// Scans through the pathname and grabs the content within that pathname and prints out the items in
//    that directory until there is nothing left to print, then it will print out a new line and exit.

void print_directory(inode_state& state) {
   cout << pathname(state.get_contents().at("."), state.get_root()->get_inode_nr()) << ":" << endl;
   for (auto content : state.get_contents()) {
      cout << "\t" << content.second->get_inode_nr() << "\t"
           << content.second->size() << " "
           << content.first << " "
           << endl;
   }
   cout << endl;
}
