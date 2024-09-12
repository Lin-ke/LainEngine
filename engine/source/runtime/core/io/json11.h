
#pragma once

#include "core/templates/vector.h"
#include "core/templates/hash_map.h"

#include <initializer_list>
#ifdef _MSC_VER
    #if _MSC_VER <= 1800 // VS 2013
        #ifndef noexcept
            #define noexcept throw()
        #endif

        #ifndef snprintf
            #define snprintf _snprintf_s
        #endif
    #endif
#endif

namespace lain {

enum JsonParse {
    STANDARD, COMMENTS
};

class JsonValue;

class Json final {
public:
    // Types
    enum Type {
        NUL, NUMBER, BOOL, STRING, ARRAY, OBJECT
    };

    // Array and object typedefs
    typedef Vector<Json> array;
    typedef HashMap<String, Json> object;

    // Constructors for the various types of JSON value.
    Json() noexcept;                // NUL
    Json(std::nullptr_t) noexcept;  // NUL
    Json(double value);             // NUMBER
    Json(int64_t value);                // NUMBER
    Json(int value);
    Json(bool value);               // BOOL
    Json(const String &value);      // STRING
    Json(String &&value);            // STRING
    Json(const char * value);       // STRING
    Json(const char32_t* value);
    Json(const array &values);      // ARRAY
    Json(array &&values);           // ARRAY
    Json(const object &values);     // OBJECT
    Json(object &&values);          // OBJECT

    // Implicit constructor: anything with a to_json() function.
    template <class T, class = decltype(&T::to_json)>
    Json(const T & t) : Json(t.to_json()) {}

    // This prevents Json(some_pointer) from accidentally producing a bool. Use
    // Json(bool(some_pointer)) if that behavior is desired.
    Json(void *) = delete;

    // Accessors
    Type type() const;

    bool is_null()   const { return type() == NUL; }
    bool is_number() const { return type() == NUMBER; }
    bool is_bool()   const { return type() == BOOL; }
    bool is_string() const { return type() == STRING; }
    bool is_array()  const { return type() == ARRAY; }
    bool is_object() const { return type() == OBJECT; }

    // Return the enclosed value if this is a number, 0 otherwise. Note that json11 does not
    // distinguish between integer and non-integer numbers - number_value() and int_value()
    // can both be applied to a NUMBER-typed object.
    double number_value() const;
    int64_t int_value() const;

    // Return the enclosed value if this is a boolean, false otherwise.
    bool bool_value() const;
    // Return the enclosed string if this is a string, "" otherwise.
    const String &string_value() const;
    // Return the enclosed std::vector if this is an array, or an empty vector otherwise.
    const array &array_items() const;
    // Return the enclosed std::map if this is an object, or an empty map otherwise.
    const object &object_items() const;

    // Return a reference to arr[i] if this is an array, Json() otherwise.
    const Json & operator[](int i) const;
    // Return a reference to obj[key] if this is an object, Json() otherwise.
    const Json & operator[](const String &key) const;

    // Serialize.
    void dump(String &out) const;
    String dump() const {
        String out;
        dump(out);
        return out;
    }

    // Parse. If parse fails, return Json() and assign an error message to err.
    static Json parse(const String & in,
                      String & err,
                      JsonParse strategy = JsonParse::STANDARD);
    //static Json parse(const String& in, String& error, JsonParse strategy = JsonParse::STANDARD);
    static Json parse(const char * in,
                      String & err,
                      JsonParse strategy = JsonParse::STANDARD) {
        if (in) {
            return parse(String(in), err, strategy);
        } else {
            err = "null input";
            return nullptr;
        }
    }
    // Parse multiple objects, concatenated or separated by whitespace
    static Vector<Json> parse_multi(
        const String & in,
        int64_t & parser_stop_pos,
        String & err,
        JsonParse strategy = JsonParse::STANDARD);


    bool operator== (const Json &rhs) const;
    bool operator<  (const Json &rhs) const;
    bool operator!= (const Json &rhs) const { return !(*this == rhs); }
    bool operator<= (const Json &rhs) const { return !(rhs < *this); }
    bool operator>  (const Json &rhs) const { return  (rhs < *this); }
    bool operator>= (const Json &rhs) const { return !(*this < rhs); }

    /* has_shape(types, err)
     *
     * Return true if this is a JSON object and, for each item in types, has a field of
     * the given type. If not, return false and set err to a descriptive message.
     */
    typedef std::initializer_list<std::pair<String, Type>> shape;
    bool has_shape(const shape & types, String & err) const;

private:
    std::shared_ptr<JsonValue> m_ptr;
};

// Internal class hierarchy - JsonValue objects are not exposed to users of this API.
class JsonValue {
protected:
    friend class Json;
    friend class JsonInt;
    friend class JsonDouble;
    virtual Json::Type type() const = 0;
    virtual bool equals(const JsonValue * other) const = 0;
    virtual bool less(const JsonValue * other) const = 0;
    virtual void dump(String &out) const = 0;
    virtual double number_value() const;
    virtual int64_t int_value() const;
    virtual bool bool_value() const;
    virtual const String &string_value() const;
    virtual const Json::array &array_items() const;
    virtual const Json &operator[](int i) const;
    virtual const Json::object &object_items() const;
    virtual const Json &operator[](const String &key) const;
    virtual ~JsonValue() {}
};

} // namespace json11
