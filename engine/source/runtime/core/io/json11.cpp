/* Copyright (c) 2013 Dropbox, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "json11.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>

#define max_depth 200
namespace lain {

using string = String;
using std::make_shared;
using std::move;

/* Helper for representing null - just a do-nothing struct, plus comparison
 * operators so the helpers in JsonValue work. We can't use nullptr_t because
 * it may not be orderable.
 */
struct NullStruct {
  bool operator==(NullStruct) const { return true; }
  bool operator<(NullStruct) const { return false; }
};

/* * * * * * * * * * * * * * * * * * * *
 * Serialization
 */

static void dump(NullStruct, string& out) {
  out += "null";
}

static void dump(double value, string& out) {
  out += String::num(value, 14 - (int)floor(log10(value)));
}

static void dump(int64_t value, string& out) {
  out += itos(value);
}

static void dump(bool value, string& out) {
  out += value ? "true" : "false";
}

static void dump(const string& value, string& out) {
  out += '"';
  out += value.json_escape();  // 这个学会了， xml有 xml_escape
  out += '"';
}

static void dump(const Json::array& values, string& out) {
  bool first = true;
  out += "[";
  for (const auto& value : values) {
    if (!first)
      out += ", ";
    value.dump(out);
    first = false;
  }
  out += "]";
}

static void dump(const Json::object& values, string& out) {
  bool first = true;
  out += "{";
  for (const auto& kv : values) {
    if (!first)
      out += ", ";
    dump(kv.key, out);
    out += ": ";
    kv.value.dump(out);
    first = false;
  }
  out += "}";
}

void Json::dump(string& out) const {
  m_ptr->dump(out);
}

/* * * * * * * * * * * * * * * * * * * *
 * Value wrappers
 */

template <Json::Type tag, typename T>
class Value : public JsonValue {
 protected:
  // Constructors
  explicit Value(const T& value) : m_value(value) {}
  explicit Value(T&& value) : m_value(move(value)) {}

  // Get type tag
  Json::Type type() const override { return tag; }

  // Comparisons
  bool equals(const JsonValue* other) const override { return false; }
  bool less(const JsonValue* other) const override { return true; } // 没有判断Value<tag, T> 是否有operator<

  const T m_value;
  void dump(string& out) const override { lain::dump(m_value, out); }
};

class JsonDouble final : public Value<Json::NUMBER, double> {
  double number_value() const override { return m_value; }
  int64_t int_value() const override { return static_cast<int64_t>(m_value); }
  bool equals(const JsonValue* other) const override { return m_value == other->number_value(); }
  bool less(const JsonValue* other) const override { return m_value < other->number_value(); }

 public:
  explicit JsonDouble(double value) : Value(value) {}
};

class JsonInt final : public Value<Json::NUMBER, int64_t> {
  double number_value() const override { return m_value; }
  int64_t int_value() const override { return m_value; }
  bool equals(const JsonValue* other) const override { return m_value == other->number_value(); }
  bool less(const JsonValue* other) const override { return m_value < other->number_value(); }

 public:
  explicit JsonInt(int64_t value) : Value(value) {}
};

class JsonBoolean final : public Value<Json::BOOL, bool> {
  bool bool_value() const override { return m_value; }

 public:
  explicit JsonBoolean(bool value) : Value(value) {}
};

class JsonString final : public Value<Json::STRING, string> {
  const string& string_value() const override { return m_value; }

 public:
  explicit JsonString(const string& value) : Value(value) {}
  explicit JsonString(string&& value) : Value(move(value)) {}
};

class JsonArray final : public Value<Json::ARRAY, Json::array> {
  const Json::array& array_items() const override { return m_value; }
  const Json& operator[](int i) const override;

 public:
  explicit JsonArray(const Json::array& value) : Value(value) {}
  explicit JsonArray(Json::array&& value) : Value(move(value)) {}
};

class JsonObject final : public Value<Json::OBJECT, Json::object> {
  const Json::object& object_items() const override { return m_value; }
  const Json& operator[](const string& key) const override;

 public:
  explicit JsonObject(const Json::object& value) : Value(value) {}
  explicit JsonObject(Json::object&& value) : Value(move(value)) {}
};

class JsonNull final : public Value<Json::NUL, NullStruct> {
 public:
  JsonNull() : Value({}) {}
};

/* * * * * * * * * * * * * * * * * * * *
 * Static globals - static-init-safe
 */
struct Statics {
  const std::shared_ptr<JsonValue> null = make_shared<JsonNull>();
  const std::shared_ptr<JsonValue> t = make_shared<JsonBoolean>(true);
  const std::shared_ptr<JsonValue> f = make_shared<JsonBoolean>(false);
  const string empty_string;
  const Vector<Json> empty_vector;
  const HashMap<string, Json> empty_HashMap;
  Statics() {}
};

static const Statics& statics() {
  static const Statics s{};
  return s;
}

static const Json& static_null() {
  // This has to be separate, not in Statics, because Json() accesses statics().null.
  static const Json json_null;
  return json_null;
}

/* * * * * * * * * * * * * * * * * * * *
 * Constructors
 */

Json::Json() noexcept : m_ptr(statics().null) {}
Json::Json(std::nullptr_t) noexcept : m_ptr(statics().null) {}
Json::Json(double value) : m_ptr(make_shared<JsonDouble>(value)) {}
Json::Json(int64_t value) : m_ptr(make_shared<JsonInt>(value)) {}
Json::Json(int value) : m_ptr(make_shared<JsonInt>(value)) {}
Json::Json(bool value) : m_ptr(value ? statics().t : statics().f) {}
Json::Json(const string& value) : m_ptr(make_shared<JsonString>(value)) {}
// Json::Json(string&& value) : m_ptr(make_shared<JsonString>(move(value))) {}
Json::Json(const char* value) : m_ptr(make_shared<JsonString>(value)) {}
Json::Json(const Json::array& values) : m_ptr(make_shared<JsonArray>(values)) {}
// Json::Json(Json::array&& values) : m_ptr(make_shared<JsonArray>(move(values))) {}
Json::Json(const Json::object& values) : m_ptr(make_shared<JsonObject>(values)) {}
// Json::Json(Json::object&& values) : m_ptr(make_shared<JsonObject>(move(values))) {}

/* * * * * * * * * * * * * * * * * * * *
 * Accessors
 */

Json::Type Json::type() const {
  return m_ptr->type();
}
double Json::number_value() const {
  return m_ptr->number_value();
}
int64_t Json::int_value() const {
  return m_ptr->int_value();
}
bool Json::bool_value() const {
  return m_ptr->bool_value();
}
const string& Json::string_value() const {
  return m_ptr->string_value();
}
const Vector<Json>& Json::array_items() const {
  return m_ptr->array_items();
}
const HashMap<string, Json>& Json::object_items() const {
  return m_ptr->object_items();
}
const Json& Json::operator[](int i) const {
  return (*m_ptr)[i];
}
const Json& Json::operator[](const string& key) const {
  return (*m_ptr)[key];
}

double JsonValue::number_value() const {
  return 0;
}
int64_t JsonValue::int_value() const {
  return 0;
}
bool JsonValue::bool_value() const {
  return false;
}
const string& JsonValue::string_value() const {
  return statics().empty_string;
}
const Vector<Json>& JsonValue::array_items() const {
  return statics().empty_vector;
}
const HashMap<string, Json>& JsonValue::object_items() const {
  return statics().empty_HashMap;
}
const Json& JsonValue::operator[](int) const {
  return static_null();
}
const Json& JsonValue::operator[](const string&) const {
  return static_null();
}

const Json& JsonObject::operator[](const string& key) const {
  auto iter = m_value.find(key);
  return (iter == m_value.end()) ? static_null() : iter->value;
}
const Json& JsonArray::operator[](int i) const {
  if (i >= m_value.size())
    return static_null();
  else
    return m_value[i];
}

/* * * * * * * * * * * * * * * * * * * *
 * Comparison
 */

bool Json::operator==(const Json& other) const {
  if (m_ptr == other.m_ptr)
    return true;
  if (m_ptr->type() != other.m_ptr->type())
    return false;

  return m_ptr->equals(other.m_ptr.get());
}

bool Json::operator<(const Json& other) const {
  if (m_ptr == other.m_ptr)
    return false;
  if (m_ptr->type() != other.m_ptr->type())
    return m_ptr->type() < other.m_ptr->type();

  return m_ptr->less(other.m_ptr.get());
}

/* * * * * * * * * * * * * * * * * * * *
 * Parsing
 */

/* esc(c)
 *
 * Format char c suitable for printing in an error message.
 */
static inline string esc(char c) {
  char buf[12];
  if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) <= 0x7f) {
    snprintf(buf, sizeof buf, "'%c' (%d)", c, c);
  } else {
    snprintf(buf, sizeof buf, "(%d)", c);
  }
  return string(buf);
}

static inline bool in_range(long x, long lower, long upper) {
  return (x >= lower && x <= upper);
}

namespace {
/* JsonParser
 *
 * Object that tracks all state of an in-progress parse.
 */
struct JsonParser final {

  /* State
     */
  const string& str;
  int i;
  string& err;
  bool failed;
  const JsonParse strategy;

  /* fail(msg, err_ret = Json())
     *
     * Mark this parse as failed.
     */
  Json fail(string&& msg) { return fail(move(msg), Json()); }

  template <typename T>
  T fail(string&& msg, const T err_ret) {
    if (!failed)
      err = std::move(msg);
    failed = true;
    return err_ret;
  }

  /* consume_whitespace()
     *
     * Advance until the current character is non-whitespace.
     */
  void consume_whitespace() {
    while (str[i] == ' ' || str[i] == '\r' || str[i] == '\n' || str[i] == '\t')
      i++;
  }

  /* consume_comment()
     *
     * Advance comments (c-style inline and multiline).
     */
  bool consume_comment() {
    bool comment_found = false;
    if (str[i] == '/') {
      i++;
      if (i == str.size())
        return fail("unexpected end of input after start of comment", false);
      if (str[i] == '/') {  // inline comment
        i++;
        // advance until next line, or end of input
        while (i < str.size() && str[i] != '\n') {
          i++;
        }
        comment_found = true;
      } else if (str[i] == '*') {  // multiline comment
        i++;
        if (i > str.size() - 2)
          return fail("unexpected end of input inside multi-line comment", false);
        // advance until closing tokens
        while (!(str[i] == '*' && str[i + 1] == '/')) {
          i++;
          if (i > str.size() - 2)
            return fail("unexpected end of input inside multi-line comment", false);
        }
        i += 2;
        comment_found = true;
      } else
        return fail("malformed comment", false);
    }
    return comment_found;
  }

  /* consume_garbage()
     *
     * Advance until the current character is non-whitespace and non-comment.
     */
  void consume_garbage() {
    consume_whitespace();
    if (strategy == JsonParse::COMMENTS) {
      bool comment_found = false;
      do {
        comment_found = consume_comment();
        if (failed)
          return;
        consume_whitespace();
      } while (comment_found);
    }
  }

  /* get_next_token()
     *
     * Return the next non-whitespace character. If the end of the input is reached,
     * flag an error and return 0.
     */
  char32_t get_next_token() {
    consume_garbage();
    if (failed)
      return static_cast<char32_t>(0);
    if (i == str.size())
      return fail("unexpected end of input", static_cast<char32_t>(0));

    return str[i++];
  }

  /* encode_utf8(pt, out)
     *
     * Encode pt as UTF-8 and add it to out.
     */
  void encode_utf8(long pt, string& out) {
    if (pt < 0)
      return;

    if (pt < 0x80) {
      out += static_cast<char>(pt);
    } else if (pt < 0x800) {
      out += static_cast<char>((pt >> 6) | 0xC0);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    } else if (pt < 0x10000) {
      out += static_cast<char>((pt >> 12) | 0xE0);
      out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    } else {
      out += static_cast<char>((pt >> 18) | 0xF0);
      out += static_cast<char>(((pt >> 12) & 0x3F) | 0x80);
      out += static_cast<char>(((pt >> 6) & 0x3F) | 0x80);
      out += static_cast<char>((pt & 0x3F) | 0x80);
    }
  }

  /* parse_string()
     *
     * Parse a string, starting at the current position.
     */
  string parse_string() {
    string out;
    while (true) {
      if (i == str.size())
        return fail("unexpected end of input in string", "");

      char32_t ch = str[i++];

      if (ch == '"') {
        return out;
      }
      // The usual case: non-escaped characters
      if (ch != '\\') {
        out += ch;
        continue;
      }

      // Handle escapes
      if (i == str.size())
        return fail("unexpected end of input in string", "");

      ch = str[i++];
      char32_t res = 0;
      switch (ch) {
        case 'u': {

          // Extract 4-byte escape sequence
          string esc = str.substr(i, 4);
          // Explicitly check length of the substring. The following loop
          // relies on std::string returning the terminating NUL when
          // accessing str[length]. Checking here reduces brittleness.
          if (esc.length() < 4) {
            return fail("bad \\u escape: " + esc, "");
          }

          for (int j = 0; j < 4; j++) {
            if (!in_range(esc[j], 'a', 'f') && !in_range(esc[j], 'A', 'F')  // ! is_hex_digit
                && !in_range(esc[j], '0', '9'))
              return fail("bad \\u escape: " + esc, "");
            char32_t v;
            char32_t c = esc[j];
            if (is_digit(c)) {
              v = c - '0';
            } else if (c >= 'a' && c <= 'f') {
              v = c - 'a';
              v += 10;
            } else if (c >= 'A' && c <= 'F') {
              v = c - 'A';
              v += 10;
            } else {
              ERR_PRINT("Bug parsing hex constant.");
              v = 0;
            }
            res <<= 4;
            res |= v;  // res = res * 16 + v (十六进制)
          }
          // 验证
          i += 4;
        } break;
        case 'b': {
          res = 8;
        } break;
        case 't':
          res = 9;
          break;
        case 'n':
          res = 10;
          break;
        case 'f':
          res = 12;
          break;
        case 'r':
          res = 13;
          break;
        case '"':
        case '\\':
        case '/': {
          res = ch;
        } break;
        default: {
          return fail("Invalid escape sequence.", "");
        }
      }
      out += res;
      continue;
    }
    return out;
  }

  /* parse_number()
     *
     * Parse a double.
     */
  Json parse_number() {
    int start_pos = i;

    if (str[i] == '-')
      i++;

    // Integer part
    if (str[i] == '0') {
      i++;
      if (in_range(str[i], '0', '9'))
        return fail("leading 0s not permitted in numbers");
    } else if (in_range(str[i], '1', '9')) {
      i++;
      while (in_range(str[i], '0', '9'))
        i++;
    } else {
      return fail("invalid " + esc(str[i]) + " in number");
    }

    if (str[i] != '.' && str[i] != 'e' && str[i] != 'E' && (i - start_pos) <= static_cast<int>(std::numeric_limits<int>::digits10)) {
      return ((str.substr(start_pos, i - start_pos)).to_int());
    }

    // Decimal part
    if (str[i] == '.') {
      i++;
      if (!in_range(str[i], '0', '9'))
        return fail("at least one digit required in fractional part");

      while (in_range(str[i], '0', '9'))
        i++;
    }

    // Exponent part
    if (str[i] == 'e' || str[i] == 'E') {
      i++;

      if (str[i] == '+' || str[i] == '-')
        i++;

      if (!in_range(str[i], '0', '9'))
        return fail("at least one digit required in exponent");

      while (in_range(str[i], '0', '9'))
        i++;
    }

    return (str.substr(start_pos)).to_float();
  }

  /* expect(str, res)
     *
     * Expect that 'str' starts at the character that was just read. If it does, advance
     * the input and return res. If not, flag an error.
     */
  Json expect(const string& expected, Json res) {
    assert(i != 0);
    i--;
    if (str.substr(i, expected.length()) == expected) {
      i += expected.length();
      return res;
    } else {
      return fail("parse error: expected " + expected + ", got " + str.substr(i, expected.length()));
    }
  }

  /* parse_json()
     *
     * Parse a JSON object.
     */
  Json parse_json(int64_t depth) {
    if (depth > max_depth) {
      return fail("exceeded maximum nesting depth");
    }

    char32_t ch = get_next_token();
    if (failed)
      return Json();

    if (ch == '-' || (ch >= '0' && ch <= '9')) {
      i--;
      return parse_number();
    }

    if (ch == 't')
      return expect("true", true);

    if (ch == 'f')
      return expect("false", false);

    if (ch == 'n')
      return expect("null", Json());

    if (ch == '"')
      return parse_string();

    if (ch == '{') {
      HashMap<string, Json> data;
      ch = get_next_token();
      if (ch == '}')
        return data;

      while (1) {
        if (ch != '"')
          return fail("expected '\"' in object, got " + esc(ch));

        string key = parse_string();
        if (failed)
          return Json();

        ch = get_next_token();
        if (ch != ':')
          return fail("expected ':' in object, got " + esc(ch));

        data[std::move(key)] = parse_json(depth + 1);
        if (failed)
          return Json();

        ch = get_next_token();
        if (ch == '}')
          break;
        if (ch != ',')
          return fail("expected ',' in object, got " + esc(ch));

        ch = get_next_token();
      }
      return data;
    }

    if (ch == '[') {
      Vector<Json> data;
      ch = get_next_token();
      if (ch == ']')
        return data;

      while (1) {
        i--;
        data.push_back(parse_json(depth + 1));
        if (failed)
          return Json();

        ch = get_next_token();
        if (ch == ']')
          break;
        if (ch != ',')
          return fail("expected ',' in list, got " + esc(ch));

        ch = get_next_token();
        (void)ch;
      }
      return data;
    }

    return fail("expected value, got " + esc(ch));
  }
};
}  // namespace

Json Json::parse(const string& in, string& err, JsonParse strategy) {
  JsonParser parser{in, 0, err, false, strategy};
  Json result = parser.parse_json(0);

  // Check for any trailing garbage
  parser.consume_garbage();
  if (parser.failed)
    return Json();
  if (parser.i != in.length()) // size 要+1 （包含0）
    return parser.fail("unexpected trailing " + esc(in[parser.i]));

  return result;
}

// Documented in json11.hpp
Vector<Json> Json::parse_multi(const string& in, int64_t& parser_stop_pos, string& err, JsonParse strategy) {
  JsonParser parser{in, 0, err, false, strategy};
  parser_stop_pos = 0;
  Vector<Json> json_vec;
  while (parser.i != in.size() && !parser.failed) {
    json_vec.push_back(parser.parse_json(0));
    if (parser.failed)
      break;

    // Check for another object
    parser.consume_garbage();
    if (parser.failed)
      break;
    parser_stop_pos = parser.i;
  }
  return json_vec;
}

/* * * * * * * * * * * * * * * * * * * *
 * Shape-checking
 */

bool Json::has_shape(const shape& types, string& err) const {
  if (!is_object()) {
    err = "expected JSON object, got " + dump();
    return false;
  }

  const auto& obj_items = object_items();
  for (auto& item : types) {
    const auto it = obj_items.find(item.first);
    if (it == obj_items.end() || it->value.type() != item.second) {
      err = "bad type for " + item.first + " in " + dump();
      return false;
    }
  }

  return true;
}

}  // namespace lain
#undef max_depth