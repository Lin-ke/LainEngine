#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

#include "core/string/ustring.h"
#include "core/templates/vector.h"
namespace lain {

class StringBuilder {
  uint32_t string_length = 0;

  Vector<String> strings;
  Vector<const char*> c_strings;

  // -1 means it's a Godot String
  // a natural number means C string.
  Vector<int32_t> appended_strings;

 public:
  StringBuilder& append(const String& p_string) {
    if (p_string.is_empty()) {
      return *this;
    }

    strings.push_back(p_string);
    appended_strings.push_back(-1);

    string_length += p_string.length();

    return *this;
  }
  StringBuilder& append(const char* p_cstring) {
    int32_t len = strlen(p_cstring);

    c_strings.push_back(p_cstring);
    appended_strings.push_back(len);

    string_length += len;

    return *this;
  }

  _FORCE_INLINE_ StringBuilder& operator+(const String& p_string) { return append(p_string); }

  _FORCE_INLINE_ StringBuilder& operator+(const char* p_cstring) { return append(p_cstring); }

  _FORCE_INLINE_ void operator+=(const String& p_string) { append(p_string); }

  _FORCE_INLINE_ void operator+=(const char* p_cstring) { append(p_cstring); }

  _FORCE_INLINE_ int num_strings_appended() const { return appended_strings.size(); }

  _FORCE_INLINE_ uint32_t get_string_length() const { return string_length; }

  String as_string() const {
    if (string_length == 0) {
      return "";
    }

    char32_t* buffer = memnew_arr(char32_t, string_length);

    int current_position = 0;

    int godot_string_elem = 0;
    int c_string_elem = 0;

    for (int i = 0; i < appended_strings.size(); i++) {
      if (appended_strings[i] == -1) {
        // Godot string
        const String& s = strings[godot_string_elem];

        memcpy(buffer + current_position, s.ptr(), s.length() * sizeof(char32_t));

        current_position += s.length();

        godot_string_elem++;
      } else {
        const char* s = c_strings[c_string_elem];

        for (int32_t j = 0; j < appended_strings[i]; j++) {
          buffer[current_position + j] = s[j];
        }

        current_position += appended_strings[i];

        c_string_elem++;
      }
    }

    String final_string = String(buffer, string_length);

    memdelete_arr(buffer);

    return final_string;
  }

  _FORCE_INLINE_ operator String() const { return as_string(); }

  StringBuilder() {}
};
}  // namespace lain

#endif  // STRING_BUILDER_H
