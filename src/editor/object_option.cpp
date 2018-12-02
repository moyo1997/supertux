//  SuperTux
//  Copyright (C) 2015 Hume2 <teratux.mail@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "editor/object_option.hpp"

#include <string>
#include <vector>
#include <sstream>

#include "gui/menu.hpp"
#include "util/gettext.hpp"
#include "util/writer.hpp"
#include "video/color.hpp"

namespace {

template<typename T>
std::string to_string(const T& v)
{
  std::ostringstream out;
  out << v;
  return out.str();
}

} // namespace

ObjectOption::ObjectOption(MenuItemKind ip_type, const std::string& text, const std::string& key, int flags) :
  m_type(ip_type),
  m_text(text),
  m_key(key),
  m_flags(flags)
{
}

ObjectOption::~ObjectOption()
{
}

BoolObjectOption::BoolObjectOption(const std::string& text, bool* pointer, const std::string& key,
                                   int flags) :
  ObjectOption(MN_TOGGLE, text, key, flags),
  m_pointer(pointer)
{
}

void
BoolObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_toggle(-1, m_text, m_pointer);
}

void
BoolObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, *m_pointer);
  }
}

std::string
BoolObjectOption::to_string() const
{
  return *m_pointer ? _("true") : _("false");
}

IntObjectOption::IntObjectOption(const std::string& text, int* pointer, const std::string& key,
                                 int flags) :
  ObjectOption(MN_INTFIELD, text, key, flags),
  m_pointer(pointer)
{
}

void
IntObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, *m_pointer);
  }
}

std::string
IntObjectOption::to_string() const
{
  return ::to_string(*m_pointer);
}

void
IntObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_intfield(m_text, m_pointer);
}

FloatObjectOption::FloatObjectOption(const std::string& text, float* pointer, const std::string& key,
                                     int flags) :
  ObjectOption(MN_FLOATFIELD, text, key, flags),
  m_pointer(pointer)
{
}

void
FloatObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, *m_pointer);
  }
}

std::string
FloatObjectOption::to_string() const
{
  return ::to_string(*m_pointer);
}

void
FloatObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_floatfield(m_text, m_pointer);
}

StringObjectOption::StringObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                       int flags) :
  ObjectOption(MN_TEXTFIELD, text, key, flags),
  m_pointer(pointer)
{
}

void
StringObjectOption::save(Writer& writer) const
{
  auto& value = *m_pointer;
  if (!value.empty() || (m_flags & OPTION_ALLOW_EMPTY))
  {
    if (!m_key.empty()) {
      writer.write(m_key, value);
    }
  }
}

std::string
StringObjectOption::to_string() const
{
  return *m_pointer;
}

void
StringObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_textfield(m_text, m_pointer);
}

StringSelectObjectOption::StringSelectObjectOption(const std::string& text, int* pointer,
                                                   const std::vector<std::string>& select,
                                                   const std::string& key,
                                                   int flags) :
  ObjectOption(MN_STRINGSELECT, text, key, flags),
  m_pointer(pointer),
  m_select(select)
{
}

void
StringSelectObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, *m_pointer);
  }
}

std::string
StringSelectObjectOption::to_string() const
{
  int* selected_id = static_cast<int*>(m_pointer);
  if (*selected_id >= int(m_select.size()) || *selected_id < 0) {
    return _("invalid"); //Test whether the selected ID is valid
  } else {
    return m_select[*selected_id];
  }
}

void
StringSelectObjectOption::add_to_menu(Menu& menu) const
{
  int& selected_id = *m_pointer;
  if ( selected_id >= static_cast<int>(m_select.size()) || selected_id < 0 ) {
    selected_id = 0; // Set the option to zero when not selectable
  }
  menu.add_string_select(-1, m_text, m_pointer, m_select);
}

ScriptObjectOption::ScriptObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                       int flags) :
  ObjectOption(MN_SCRIPT, text, key, flags),
  m_pointer(pointer)
{
}

void
ScriptObjectOption::save(Writer& writer) const
{
  auto& value = *m_pointer;
  if (!value.empty() || (m_flags & OPTION_ALLOW_EMPTY))
  {
    if (!m_key.empty()) {
      writer.write(m_key, value);
    }
  }
}

std::string
ScriptObjectOption::to_string() const
{
  if (!m_pointer->empty()) {
    return "...";
  }
  return "";
}

void
ScriptObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_script(m_text, m_pointer);
}

FileObjectOption::FileObjectOption(const std::string& text, std::string* pointer, const std::string& key,
                                   std::vector<std::string> filter, int flags) :
  ObjectOption(MN_FILE, text, key, flags),
  m_pointer(pointer),
  m_filter(filter)
{
}

void
FileObjectOption::save(Writer& writer) const
{
  auto& value = *m_pointer;
  if (!value.empty() || (m_flags & OPTION_ALLOW_EMPTY))
  {
    if (!m_key.empty()) {
      writer.write(m_key, value);
    }
  }
}

std::string
FileObjectOption::to_string() const
{
  return *m_pointer;
}

void
FileObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_file(m_text, m_pointer, m_filter);
}

ColorObjectOption::ColorObjectOption(const std::string& text, Color* pointer, const std::string& key,
                                     int flags) :
  ObjectOption(MN_COLOR, text, key, flags),
  m_pointer(pointer)
{
}

void
ColorObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, m_pointer->toVector());
  }
}

std::string
ColorObjectOption::to_string() const
{
  return m_pointer->to_string();
}

void
ColorObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_color(m_text, m_pointer);
}

BadGuySelectObjectOption::BadGuySelectObjectOption(const std::string& text, std::vector<std::string>* pointer, const std::string& key,
                                                   int flags) :
  ObjectOption(MN_BADGUYSELECT, text, key, flags),
  m_pointer(pointer)
{
}

void
BadGuySelectObjectOption::save(Writer& writer) const
{
  if (!m_key.empty()) {
    writer.write(m_key, *m_pointer);
  }
}

std::string
BadGuySelectObjectOption::to_string() const
{
  return ::to_string(m_pointer->size());
}

void
BadGuySelectObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_badguy_select(m_text, m_pointer);
}

RemoveObjectOption::RemoveObjectOption() :
  ObjectOption(MN_REMOVE, _("Remove"), "")
{
}

std::string
RemoveObjectOption::to_string() const
{
  return {};
}

void
RemoveObjectOption::add_to_menu(Menu& menu) const
{
  menu.add_entry(MN_REMOVE, m_text);
}

/* EOF */
