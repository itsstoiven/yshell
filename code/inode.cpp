// This program was completed using pair programming.
// Steven Cruz (sicruz@ucsc.edu)
// Partner: Jason Ou (jaou@ucsc.edu)

// $Id: inode.cpp,v 1.12 2014-07-03 13:29:57-07 - - $

// Libraries

#include <iostream>
#include <stdexcept>

// Using

using namespace std;

// Header Files

#include "debug.h"
#include "inode.h"

//
// class inode
//

int inode::next_inode_nr {1};

// Constructor

inode::inode(inode_t ini_type):inode_nr (next_inode_nr++), type (ini_type) {
   switch (type) {
   case PLAIN_INODE:
      contents = make_shared<plain_file>();
      break;
   case DIR_INODE:
      contents = make_shared<directory>();
      break;
   }
   
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}
// Accessors for class inode

// Gets inode_nr
int inode::get_inode_nr() const {
  DEBUGF ('i', "inode = " << inode_nr);
  return inode_nr;
}

// Gets the type
inode_t inode::get_type() {
   return type;
}

const map<string, inode_ptr>& inode::get_contents() {
   return  directory_ptr_of(contents)->get_dirents();
}

size_t inode::size() {
   if (type == PLAIN_INODE) {
      return plain_file_ptr_of(contents)->size();
   }
   return directory_ptr_of(contents)->size();
}

// Mutators for class inode

void inode::writefile(const wordvec& data) {
   plain_file_ptr_of(contents)->writefile(data);
}

//
// class inode_state
//

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root << ", cwd = " << state.cwd;
   return out;
}

// Constructor for inode_state

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd << ", prompt = \"" << prompt << "\"");
   root = make_shared<inode>(DIR_INODE);
   cwd = root;
   directory_ptr_of(root->contents)->set_dot(root);
   directory_ptr_of(root->contents)->set_parent(root);
}

// Accessors for class inode_state

string inode_state::readfile(const wordvec path,bool check) {
   auto i_ptr = get_cwd(); 
   if(check){auto i_ptr = get_root();} 
   auto inc = path.begin();
   for(; inc != path.end()-1 ; ++inc) {
      auto p = i_ptr->get_contents().find(*inc);
      // If no such directory in path, prints it out and exits
      if( p == i_ptr->get_contents().end() ) {
         throw yshell_exn("No such path.");
      }
   // If there is such a directory, switches directory
   i_ptr = p->second;
   }
   auto file = i_ptr->get_contents().find(*(path.end()-1));
   if(file != i_ptr->get_contents().end()){
      if(file->second->get_type() == DIR_INODE){
         throw yshell_exn("No such file.");
         return "";
      }
         return plain_file_ptr_of(file->second->contents)->readfile();
   } else {
      throw yshell_exn("No such file.");
      return "";
   }
}

// Gets the root

const inode_ptr inode_state::get_root() {
   return root;
}

// Gets the current working directory

const inode_ptr inode_state::get_cwd() {
   return cwd;
}

// Gets the prompt

const string& inode_state::get_prompt() {
   return prompt;
}

const map<string, inode_ptr>& inode_state::get_contents() {
   return  directory_ptr_of(cwd->contents)->get_dirents();
}

// Mutators for class inode_state

void inode_state::remove(const wordvec path,bool check,bool d){
   //start at root go down path
   auto i_ptr = get_cwd();
   if(check){auto i_ptr = get_root();} 
   if(path.size()==0) {root = nullptr;return;} 
   auto inc = path.begin();
   for(;inc != path.end()-1; ++inc) {
      auto p = i_ptr->get_contents().find(*inc);
      // If no such directory, then it prints and exits
      if( p == i_ptr->get_contents().end() ) {
         throw yshell_exn("No such path.");
      }
      // If such directory exists, then it switches to that directory
      i_ptr = p->second;
   }
   
   // Removes the last file in the path if it does exist
   if(i_ptr->get_contents().find(*(path.end()-1)) != i_ptr->get_contents().end()) {
      if(d && i_ptr->get_contents().find(*(path.end()-1))->second->get_type()==DIR_INODE){
         throw yshell_exn("Can't rm directory.");
      } else {
         try{
            directory_ptr_of(i_ptr->contents)->remove(*(path.end()-1)); 
         } catch(yshell_exn) {
            throw yshell_exn("No such file.");
         }
      }
   } else {
      throw yshell_exn("No such file.");
   }
}

// Changes the value of prompt

void inode_state::set_prompt(const string& p) {
   prompt = p;
}

void inode_state::add_file(const wordvec& data, const wordvec& path, bool check) {
   // Begins at the root then goes down the path
   auto i_ptr = get_cwd();
   if (check) {auto i_ptr = get_root();}
   auto inc = path.begin();
   for (; inc != path.end() - 1; ++inc) {
      auto p = i_ptr->get_contents().find(*inc);
      // If no such directory exists, then it prints and exits
      if ( p == i_ptr->get_contents().end() ) {
         throw yshell_exn("No such path.");
      }
      // If such directory exists, then it switches
      i_ptr = p->second;
   }
  
   // Creates the file in the last part of the path
   if (i_ptr->get_contents().find(*inc) != i_ptr->get_contents().end() ) {
      throw yshell_exn("File exists.");
   } else {
     directory_ptr_of(i_ptr->contents)->mkfile(*inc).writefile(data);
   }
}

void inode_state::add_directory(const wordvec path, bool check) {
   // Begins at the root and goes down the path
   auto i_ptr = get_cwd();
   if (check) {auto i_ptr = get_root();}
   auto inc = path.begin();
   for (; inc != path.end() - 1; ++inc) {
      auto p = i_ptr->get_contents().find(*inc);
      // If no such directory, then it prints our and exits
      if (p == i_ptr->get_contents().end() || p->second->get_type() == PLAIN_INODE ) {
         throw yshell_exn("No such path.");
      }
      // If such directory, then it will switch
      i_ptr = p->second;
   }
   // Creates a directory in the last path directory
   auto p = i_ptr->get_contents().find(*inc);
   if (p != i_ptr->get_contents().end() ) {
      throw yshell_exn("Directory exists");
   } else {
      auto new_dir = directory_ptr_of(i_ptr->contents)->mkdir(*inc);
      directory_ptr_of(new_dir.contents)->set_dot(make_shared<inode>(new_dir));
      directory_ptr_of(new_dir.contents)->set_parent(i_ptr);
   }
}

void inode_state::set_cwd(const wordvec path, const bool check) {
   // Begins at the root and goes down the path
   auto i_ptr = cwd;
   if (check) {i_ptr = root;}
   auto inc = path.begin();
   for (; inc != path.end(); ++inc) {
      auto p = i_ptr->get_contents().find(*inc);
      // If no such directory exists, then it will print and exit out
      if ( p == i_ptr->get_contents().end() ) {
         throw yshell_exn("No such path");
      }
      // If it does exist, then it will switch
      i_ptr = p->second;
   }
   cwd = i_ptr;
}

//
// class plain_file
//

size_t plain_file::size() const {
   size_t size {};
   for (auto word : data) {
      size += word.size();
   }
   if (data.size() != 0) {
      size = size + data.size() - 1 ;
   }
   DEBUGF ('i', "size = " << size);
   return size;
}

// Accessors for plain_file

string plain_file::readfile() {
   string str = ""; 
   for(auto elem : data){
      str = str + elem+ " ";
   }
   return str;
}

// Mutator for plain_file

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data = words;
}

//
// class directory
//

// Constructor for class directory

// Each directory begins with a "." and ".." directory on create

directory::directory() {
   dirents.insert({".", nullptr}); dirents.insert({"..", nullptr});
}

// Accessors for class directory

size_t directory::size() const {
   size_t size {dirents.size()};
   DEBUGF ('i', "size = " << size);
   return size;
}

const map<string, inode_ptr>& directory::get_dirents() {
   return dirents;
}

inode& directory::mkdir(const string& dirname) {
   inode_ptr inc = make_shared<inode>(DIR_INODE);
   if (dirents.find(dirname) == dirents.end()) {
      dirents.insert({dirname, inc});
   }
   return *inc;
}

inode& directory::mkfile(const string& filename) {
   inode_ptr inc = make_shared<inode>(PLAIN_INODE);
   if(dirents.find(filename) == dirents.end()) {
      dirents.insert({filename, inc});
   }
   return *inc;
}

// Mutators for class directory

void directory::remove (const string& filename) {
   auto elem = dirents.find(filename);
   if(elem != dirents.end()){
      (*elem).second == nullptr;
      dirents.erase(elem);
   } else {
      throw yshell_exn("No such file or directory.");
   }
     
   DEBUGF ('i', filename);
}

void directory::set_dot(inode_ptr ip) {
   dirents.at(".") = ip;
}

void directory::set_parent(inode_ptr ip) {
   dirents.at("..") = ip;
}

//
// class file_base
//

plain_file_ptr plain_file_ptr_of (file_base_ptr ptr) {
  plain_file_ptr pfptr = dynamic_pointer_cast<plain_file> (ptr);
  if (pfptr == nullptr) throw invalid_argument ("plain_file_ptr_of");
  return pfptr;
}

directory_ptr directory_ptr_of (file_base_ptr ptr) {
   directory_ptr dirptr = dynamic_pointer_cast<directory> (ptr);
   if (dirptr == nullptr) throw invalid_argument ("directory_ptr_of");
   return dirptr;
}
