//  $Id$
//
//  TinyGetText
//  Copyright (C) 2006 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <config.h>

#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <ctype.h>
#include <errno.h>

#include <SDL.h>

#ifndef GP2X
#include <SDL_stdinc.h>
#endif

#include "tinygettext.hpp"
#include "log.hpp"
#include "physfs/physfs_stream.hpp"
#include "findlocale.hpp"

//#define TRANSLATION_DEBUG

namespace TinyGetText {

/** Convert \a which is in \a from_charset to \a to_charset and return it */
std::string convert(const std::string& text,
                    const std::string& from_charset,
                    const std::string& to_charset)
{
#ifndef GP2X
  if (from_charset == to_charset)
    return text;

  char *in = new char[text.length() + 1];
  strcpy(in, text.c_str());
  char *out = SDL_iconv_string(to_charset.c_str(), from_charset.c_str(), in, text.length() + 1);
  delete[] in; 
  if(out == 0)
  {
    log_warning << "Error: conversion from " << from_charset << " to " << to_charset << " failed" << std::endl;
    return "";
  }
  std::string ret(out);
  SDL_free(out);
  return ret;
#else
  log_warning << "FIXME: Char conversion not supported on GP2X!" << std::endl;
  return "";
#endif
#if 0
  iconv_t cd = SDL_iconv_open(to_charset.c_str(), from_charset.c_str());

  size_t in_len = text.length();
  size_t out_len = text.length()*3; // FIXME: cross fingers that this is enough

  char*  out_orig = new char[out_len];
  char*  in_orig  = new char[in_len+1];
  strcpy(in_orig, text.c_str());

  char* out = out_orig;
  ICONV_CONST char* in  = in_orig;
  size_t out_len_temp = out_len; // iconv is counting down the bytes it has
                                 // written from this...

  size_t retval = SDL_iconv(cd, &in, &in_len, &out, &out_len_temp);
  out_len -= out_len_temp; // see above
  if (retval == (size_t) -1)
    {
      log_warning << strerror(errno) << std::endl;
      log_warning << "Error: conversion from " << from_charset << " to " << to_charset << " went wrong: " << retval << std::endl;
      return "";
    }
  SDL_iconv_close(cd);

  std::string ret(out_orig, out_len);
  delete[] out_orig;
  delete[] in_orig;
  return ret;
#endif
}

bool has_suffix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(lhs.length() - rhs.length(), rhs.length(), rhs) == 0;
}

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

int plural1(int )     { return 0; }
int plural2_1(int n)  { return (n != 1); }
int plural2_2(int n)  { return (n > 1); }
int plural3_lv(int n) { return (n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2); }
int plural3_ga(int n) { return n==1 ? 0 : n==2 ? 1 : 2; }
int plural3_lt(int n) { return (n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_1(int n)  { return (n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_sk(int n) { return (n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2; }
int plural3_pl(int n) { return (n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2); }
int plural3_sl(int n) { return (n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3); }

/** Language Definitions */
//*{
LanguageDef lang_hu("hu", "Hungarian",         1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_ja("ja", "Japanese",          1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_ko("ko", "Korean",            1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_tr("tr", "Turkish",           1, plural1); // "nplurals=1; plural=0;"
LanguageDef lang_da("da", "Danish",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nl("nl", "Dutch",             2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_en("en", "English",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fo("fo", "Faroese",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_de("de", "German",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nb("nb", "Norwegian Bokmal",  2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_no("no", "Norwegian",         2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_nn("nn", "Norwegian Nynorsk", 2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_sv("sv", "Swedish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_et("et", "Estonian",          2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fi("fi", "Finnish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_el("el", "Greek",             2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_he("he", "Hebrew",            2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_it("it", "Italian",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_pt("pt", "Portuguese",        2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_es("es", "Spanish",           2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_eo("eo", "Esperanto",         2, plural2_1); // "nplurals=2; plural=(n != 1);"
LanguageDef lang_fr("fr", "French",            2, plural2_2); // "nplurals=2; plural=(n > 1);"
LanguageDef lang_pt_BR("pt_BR", "Brazilian",   2, plural2_2); // "nplurals=2; plural=(n > 1);"
LanguageDef lang_lv("lv", "Latvian",           3, plural3_lv); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2);"
LanguageDef lang_ga("ga", "Irish",             3, plural3_ga); // "nplurals=3; plural=n==1 ? 0 : n==2 ? 1 : 2;"
LanguageDef lang_lt("lt", "Lithuanian",        3, plural3_lt); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_hr("hr", "Croatian",          3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_cs("cs", "Czech",             3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_ru("ru", "Russian",           3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_uk("uk", "Ukrainian",         3, plural3_1); // "nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);"
LanguageDef lang_sk("sk", "Slovak",            3, plural3_sk); // "nplurals=3; plural=(n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2;"
LanguageDef lang_pl("pl", "Polish",            3, plural3_pl); // "nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
LanguageDef lang_sl("sl", "Slovenian",         3, plural3_sl); // "nplurals=4; plural=(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3);"
//*}

LanguageDef&
get_language_def(const std::string& name)
{
  if (name == "hu") return lang_hu;
  else if (name == "ja") return lang_ja;
  else if (name == "ko") return lang_ko;
  else if (name == "tr") return lang_tr;
  else if (name == "da") return lang_da;
  else if (name == "nl") return lang_nl;
  else if (name == "en") return lang_en;
  else if (name == "fo") return lang_fo;
  else if (name == "de") return lang_de;
  else if (name == "nb") return lang_nb;
  else if (name == "no") return lang_no;
  else if (name == "nn") return lang_nn;
  else if (name == "sv") return lang_sv;
  else if (name == "et") return lang_et;
  else if (name == "fi") return lang_fi;
  else if (name == "el") return lang_el;
  else if (name == "he") return lang_he;
  else if (name == "it") return lang_it;
  else if (name == "pt") return lang_pt;
  else if (name == "es") return lang_es;
  else if (name == "eo") return lang_eo;
  else if (name == "fr") return lang_fr;
  else if (name == "pt_BR") return lang_pt_BR;
  else if (name == "lv") return lang_lv;
  else if (name == "ga") return lang_ga;
  else if (name == "lt") return lang_lt;
  else if (name == "hr") return lang_hr;
  else if (name == "cs") return lang_cs;
  else if (name == "ru") return lang_ru;
  else if (name == "uk") return lang_uk;
  else if (name == "sk") return lang_sk;
  else if (name == "pl") return lang_pl;
  else if (name == "sl") return lang_sl;
  else return lang_en;
}

DictionaryManager::DictionaryManager()
  : current_dict(&empty_dict)
{
  parseLocaleAliases();
  // Environment variable SUPERTUX_LANG overrides language settings.
  const char* lang = getenv( "SUPERTUX_LANG" );
  if( lang ){
    set_language( lang );
    return;
  }
  // use findlocale to setup language
  FL_Locale *locale;
  FL_FindLocale( &locale, FL_MESSAGES );
  if(locale->lang) {
    if (locale->country) {
      set_language( std::string(locale->lang)+"_"+std::string(locale->country) );
    } else {
      set_language( std::string(locale->lang) );
    }
  }
  FL_FreeLocale( &locale );
}

void
DictionaryManager::parseLocaleAliases()
{
  // try to parse language alias list
  std::ifstream in("/usr/share/locale/locale.alias");

  char c = ' ';
  while(in.good() && !in.eof()) {
    while(isspace(static_cast<unsigned char>(c)) && !in.eof())
      in.get(c);

    if(c == '#') { // skip comments
      while(c != '\n' && !in.eof())
        in.get(c);
      continue;
    }

    std::string alias;
    while(!isspace(static_cast<unsigned char>(c)) && !in.eof()) {
      alias += c;
      in.get(c);
    }
    while(isspace(static_cast<unsigned char>(c)) && !in.eof())
      in.get(c);
    std::string language;
    while(!isspace(static_cast<unsigned char>(c)) && !in.eof()) {
      language += c;
      in.get(c);
    }

    if(in.eof())
      break;
    set_language_alias(alias, language);
  }
}

Dictionary&
DictionaryManager::get_dictionary(const std::string& spec)
{

  //log_debug << "Dictionary for language \"" << spec << "\" requested" << std::endl;

  std::string lang = get_language_from_spec(spec);

  //log_debug << "...normalized as \"" << lang << "\"" << std::endl;

  Dictionaries::iterator i = dictionaries.find(get_language_from_spec(lang));
  if (i != dictionaries.end())
    {
      return i->second;
    }
  else // Dictionary for languages lang isn't loaded, so we load it
    {
      //log_debug << "get_dictionary: " << lang << std::endl;
      Dictionary& dict = dictionaries[lang];

      dict.set_language(get_language_def(lang));
      if(charset != "")
        dict.set_charset(charset);

      for (SearchPath::iterator p = search_path.begin(); p != search_path.end(); ++p)
        {
          char** files = PHYSFS_enumerateFiles(p->c_str());
          if(!files)
            {
              log_warning << "Error: enumerateFiles() failed on " << *p << std::endl;
            }
          else
            {
              for(const char* const* filename = files;
                      *filename != 0; filename++) {

                // check if filename matches requested language
                std::string fname = std::string(*filename);
                std::string load_from_file = "";
                if(fname == lang + ".po") {
                  load_from_file = fname;
                } else {
                  std::string::size_type s = lang.find("_");
                  if(s != std::string::npos) {
                    std::string lang_short = std::string(lang, 0, s);
                    if (fname == lang_short + ".po") {
                      load_from_file = lang_short;
                    }
                  }
                }

                // if it matched, load dictionary
                if (load_from_file != "") {
                  //log_debug << "Loading dictionary for language \"" << lang << "\" from \"" << filename << "\"" << std::endl;
                  std::string pofile = *p + "/" + *filename;
                  try {
                      IFileStream in(pofile);
                      read_po_file(dict, in);
                  } catch(std::exception& e) {
                      log_warning << "Error: Failure file opening: " << pofile << std::endl;
                      log_warning << e.what() << "" << std::endl;
                  }
                }

              }
              PHYSFS_freeList(files);
            }
        }

      return dict;
    }
}

std::set<std::string>
DictionaryManager::get_languages()
{
  std::set<std::string> languages;

  for (SearchPath::iterator p = search_path.begin(); p != search_path.end(); ++p)
    {
      char** files = PHYSFS_enumerateFiles(p->c_str());
      if (!files)
        {
          log_warning << "Error: opendir() failed on " << *p << std::endl;
        }
      else
        {
          for(const char* const* file = files; *file != 0; file++) {
              if(has_suffix(*file, ".po")) {
                  std::string filename = *file;
                  languages.insert(filename.substr(0, filename.length()-3));
              }
          }
          PHYSFS_freeList(files);
        }
    }
  return languages;
}

void
DictionaryManager::set_language(const std::string& lang)
{
  //log_debug << "set_language \"" << lang << "\"" << std::endl;
  language = get_language_from_spec(lang);
  //log_debug << "==> \"" << language << "\"" << std::endl;
  current_dict = & (get_dictionary(language));
}

const std::string&
DictionaryManager::get_language() const
{
  return language;
}

void
DictionaryManager::set_charset(const std::string& charset)
{
  dictionaries.clear(); // changing charset invalidates cache
  this->charset = charset;
  set_language(language);
}

void
DictionaryManager::set_language_alias(const std::string& alias,
    const std::string& language)
{
  language_aliases.insert(std::make_pair(alias, language));
}

std::string
DictionaryManager::get_language_from_spec(const std::string& spec)
{
  std::string lang = spec;
  Aliases::iterator i = language_aliases.find(lang);
  if(i != language_aliases.end()) {
    lang = i->second;
  }

  std::string::size_type s = lang.find(".");
  if(s != std::string::npos) {
    lang = std::string(lang, 0, s);
  }

  s = lang.find("_");
  if(s == std::string::npos) {
    std::string lang_big = lang;
    std::transform (lang_big.begin(), lang_big.end(), lang_big.begin(), toupper);
    lang += "_" + lang_big;
  }

  return lang;

}

void
DictionaryManager::add_directory(const std::string& pathname)
{
  dictionaries.clear(); // adding directories invalidates cache
  search_path.push_back(pathname);
  set_language(language);
}

//---------------------------------------------------------------------------

Dictionary::Dictionary(const LanguageDef& language_, const std::string& charset_)
  : language(language_), charset(charset_)
{
}

Dictionary::Dictionary()
  : language(lang_en)
{
}

std::string
Dictionary::get_charset() const
{
  return charset;
}

void
Dictionary::set_charset(const std::string& charset_)
{
  charset = charset_;
}

void
Dictionary::set_language(const LanguageDef& lang)
{
  language = lang;
}

std::string
Dictionary::translate(const std::string& msgid, const std::string& msgid2, int num)
{
  PluralEntries::iterator i = plural_entries.find(msgid);
  std::map<int, std::string>& msgstrs = i->second;

  if (i != plural_entries.end() && !msgstrs.empty())
    {
      int g = language.plural(num);
      std::map<int, std::string>::iterator j = msgstrs.find(g);
      if (j != msgstrs.end())
        {
          return j->second;
        }
      else
        {
          // Return the first translation, in case we can't translate the specific number
          return msgstrs.begin()->second;
        }
    }
  else
    {
#ifdef TRANSLATION_DEBUG
      log_warning << "Couldn't translate: " << msgid << std::endl;
      log_warning << "Candidates: " << std::endl;
      for (PluralEntries::iterator i = plural_entries.begin(); i != plural_entries.end(); ++i)
        log_debug << "'" << i->first << "'" << std::endl;
#endif

      if (plural2_1(num)) // default to english rules
        return msgid2;
      else
        return msgid;
    }
}

const char*
Dictionary::translate(const char* msgid)
{
  Entries::iterator i = entries.find(msgid);
  if (i != entries.end() && !i->second.empty())
    {
      return i->second.c_str();
    }
  else
    {
#ifdef TRANSLATION_DEBUG
      log_warning << "Couldn't translate: " << msgid << std::endl;
#endif
      return msgid;
    }
}

std::string
Dictionary::translate(const std::string& msgid)
{
  Entries::iterator i = entries.find(msgid);
  if (i != entries.end() && !i->second.empty())
    {
      return i->second;
    }
  else
    {
#ifdef TRANSLATION_DEBUG
      log_warning << "Couldn't translate: " << msgid << std::endl;
#endif
      return msgid;
    }
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& ,
                            const std::map<int, std::string>& msgstrs)
{
  // Do we need msgid2 for anything? its after all supplied to the
  // translate call, so we just throw it away
  plural_entries[msgid] = msgstrs;
}

void
Dictionary::add_translation(const std::string& msgid, const std::string& msgstr)
{
  entries[msgid] = msgstr;
}

class POFileReader
{
private:
  Dictionary& dict;
  std::istream& in;

  std::string from_charset;
  std::string to_charset;

  int line_num;
  int c; //TODO: char c? unsigned char c?
  enum Token {
      TOKEN_KEYWORD, //msgstr, msgid, etc.
      TOKEN_CONTENT, //string literals, concatenated ("" "foo\n" "bar\n" -> "foo\nbar\n")
      TOKEN_EOF      //ran out of tokens
  };
  Token token;
  std::string tokenContent; //current contents of the keyword or string literal(s)

public:
  POFileReader(std::istream& in_, Dictionary& dict_)
    : in(in_), dict(dict_)
  {
    line_num = 0;
    nextChar();
    if(c == 0xef) { // skip UTF-8 intro that some text editors produce
        nextChar();
        nextChar();
        nextChar();
    }
    tokenize_po();
  }

  void parse_header(const std::string& header)
  {
    // Separate the header in lines
    typedef std::vector<std::string> Lines;
    Lines lines;

    std::string::size_type start = 0;
    for(std::string::size_type i = 0; i < header.length(); ++i)
      {
        if (header[i] == '\n')
          {
            lines.push_back(header.substr(start, i - start));
            start = i+1;
          }
      }

    for(Lines::iterator i = lines.begin(); i != lines.end(); ++i)
      {
        if (has_prefix(*i, "Content-Type: text/plain; charset=")) {
          from_charset = i->substr(strlen("Content-Type: text/plain; charset="));
        }
      }

    if (from_charset.empty() || from_charset == "CHARSET")
      {
        log_warning << "Error: Charset not specified for .po, fallback to ISO-8859-1" << std::endl;
        from_charset = "ISO-8859-1";
      }

    to_charset = dict.get_charset();
    if (to_charset.empty())
      { // No charset requested from the dict, use utf-8
        to_charset = "utf-8";
        dict.set_charset(from_charset);
      }
  }

  inline void nextChar()
  {
    c = in.get();
    if (c == '\n')
      line_num++;
  }

  inline void skipSpace()
  {
    if(c == EOF)
      return;

    while(c == '#' || isspace(static_cast<unsigned char>(c))) {
      if(c == '#') {
        while(c != '\n' && c != EOF) nextChar();
      }
      nextChar();
    }
  }

  inline bool expectToken(std::string type, Token wanted) {
     if(token != wanted) {
        log_warning << "Expected " << type << ", got ";
        if(token == TOKEN_EOF)
          log_warning << "EOF";
        else if(token == TOKEN_KEYWORD)
          log_warning << "keyword '" << tokenContent << "'";
        else
          log_warning << "string \"" << tokenContent << '"';

        log_warning << " at line " << line_num << std::endl;
        return false;
     }
     return true;
  }

  inline bool expectContent(std::string type, std::string wanted) {
     if(tokenContent != wanted) {
        log_warning << "Expected " << type << ", got ";
        if(token == TOKEN_EOF)
          log_warning << "EOF";
        else if(token == TOKEN_KEYWORD)
          log_warning << "keyword '" << tokenContent << "'";
        else
          log_warning << "string \"" << tokenContent << '"';

        log_warning << " at line " << line_num << std::endl;
        return false;
     }
     return true;
  }

  void tokenize_po()
    {
      while((token = nextToken()) != TOKEN_EOF)
        {
          if(!expectToken("'msgid' keyword", TOKEN_KEYWORD) || !expectContent("'msgid' keyword", "msgid")) break;

          token = nextToken();
          if(!expectToken("name after msgid", TOKEN_CONTENT)) break;
          std::string current_msgid = tokenContent;

          token = nextToken();
          if(!expectToken("msgstr or msgid_plural", TOKEN_KEYWORD)) break;
          if(tokenContent == "msgid_plural")
            {
              //Plural form
              token = nextToken();
              if(!expectToken("msgid_plural content", TOKEN_CONTENT)) break;
              std::string current_msgid_plural = tokenContent;

              std::map<int, std::string> msgstr_plural;
              while((token = nextToken()) == TOKEN_KEYWORD && has_prefix(tokenContent, "msgstr["))
                {
                  int num;
                  if (sscanf(tokenContent.c_str(), "msgstr[%d]", &num) != 1)
                    {
                      log_warning << "Error: Couldn't parse: " << tokenContent << std::endl;
                    }

                  token = nextToken();
                  if(!expectToken("msgstr[x] content", TOKEN_CONTENT)) break;
                  msgstr_plural[num] = convert(tokenContent, from_charset, to_charset);
                }
              dict.add_translation(current_msgid, current_msgid_plural, msgstr_plural);
            }
          else
            {
              // "Ordinary" translation
              if(!expectContent("'msgstr' keyword", "msgstr")) break;

              token = nextToken();
              if(!expectToken("translation in msgstr", TOKEN_CONTENT)) break;

              if (current_msgid == "")
                { // .po Header is hidden in the msgid with the empty string
                  parse_header(tokenContent);
                }
              else
                {
                  dict.add_translation(current_msgid, convert(tokenContent, from_charset, to_charset));
                }
            }
        }
    }

  Token nextToken()
  {
    //Clear token contents
    tokenContent = "";

    skipSpace();

    if(c == EOF)
      return TOKEN_EOF;
    else if(c != '"')
      {
        // Read a keyword
        do {
          tokenContent += c;
          nextChar();
        } while(c != EOF && !isspace(static_cast<unsigned char>(c)));
        return TOKEN_KEYWORD;
      }
    else
      {
        do {
          nextChar();
          // Read content
          while(c != EOF && c != '"') {
            if (c == '\\') {
              nextChar();
              if (c == 'n') c = '\n';
              else if (c == 't') c = '\t';
              else if (c == 'r') c = '\r';
              else if (c == '"') c = '"';
              else if (c == '\\') c = '\\';
              else
                {
                  log_warning << "Unhandled escape character: " << char(c) << std::endl;
                  c = ' ';
                }
            }
            tokenContent += c;
            nextChar();
          }
          if(c == EOF) {
            log_warning << "Unclosed string literal: " << tokenContent << std::endl;
            return TOKEN_CONTENT;
          }

          // Read more strings?
          skipSpace();
        } while(c == '"');
        return TOKEN_CONTENT;
      }
  }
};

void read_po_file(Dictionary& dict_, std::istream& in)
{
  POFileReader reader(in, dict_);
}

} // namespace TinyGetText

/* EOF */
