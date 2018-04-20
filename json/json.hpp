

#pragma once

#include "myspace/_/stdafx.hpp"

MYSPACE_BEGIN

namespace jsonimpl {
class JsonValue;
}

class Json {
public:
  MYSPACE_EXCEPTION_DEFINE(Exception, myspace::Exception)
  MYSPACE_EXCEPTION_DEFINE(ParseError, Json::Exception)
  MYSPACE_EXCEPTION_DEFINE(RangeError, Json::Exception)
  MYSPACE_EXCEPTION_DEFINE(TypeError, Json::Exception)

public:
  enum Type { STR, OBJ, NUM, NUL, ARR, BOL };

  typedef std::map<std::string, Json> Object;
  typedef std::deque<Json> Array;

public:
  static Json parse(const std::string &src) noexcept(false);
  // Implicit constructor: map-like objects (std::map, std::unordered_map,
  // etc)
  template <
      class M,
      typename std::enable_if<
          std::is_constructible<
              std::string, decltype(std::declval<M>().begin()->first)>::value &&
              std::is_constructible<
                  Json, decltype(std::declval<M>().begin()->second)>::value,
          int>::type = 0>
  Json(const M &m) : Json(Object(m.begin(), m.end())) {}
  Json(std::initializer_list<std::pair<std::string, Json>> init);

  // Implicit constructor: vector-like objects (std::list, std::vector,
  // std::set, etc)
  template <class V, typename std::enable_if<
                         std::is_constructible<
                             Json, decltype(*std::declval<V>().begin())>::value,
                         int>::type = 0>
  Json(const V &v) : Json(Array(v.begin(), v.end())) {}
  Json();
  Json(const Array &);
  Json(Array &&);
  Json(const Object &);
  Json(Object &&);
  Json(bool);
  Json(double);
  Json(float);
  Json(int16_t);
  Json(uint16_t);
  Json(int32_t);
  Json(uint32_t);
  Json(int64_t);
  Json(uint64_t);
  Json(const std::string &);
  Json(std::string &&);
  Json(const char *);
  Json(Json &&);
  Json(const Json &);
  void swap(Json &);
  void copy(const Json &);
  void share(Json &);
  Json &operator=(const Json &);
  Json &operator=(Json &&);

  std::string dump() const;
  std::string toString() const;
  std::string to_json() const;

  // map like access
  Json &operator[](const std::string &);
  const Json &operator[](const std::string &) const noexcept(false);
  template <size_t N> Json &operator[](const char (&)[N]);
  template <size_t N>
  const Json &operator[](const char (&)[N]) const noexcept(false);
  // array like access
  Json &operator[](size_t) noexcept(false);
  const Json &operator[](size_t) const noexcept(false);
  // access
  std::string &stringValue() noexcept(false);
  Array &arrayValue() noexcept(false);
  Object &objectValue() noexcept(false);
  double &numberValue() noexcept(false);
  bool &boolValue() noexcept(false);
  const std::string &stringValue() const noexcept(false);
  const Array &arrayValue() const noexcept(false);
  const Object &objectValue() const noexcept(false);
  const double &numberValue() const noexcept(false);
  const bool &boolValue() const noexcept(false);
  operator std::string &() noexcept(false);
  operator double &() noexcept(false);
  operator bool &() noexcept(false);
  operator Json::Array &() noexcept(false);
  operator Json::Object &() noexcept(false);
  operator const std::string &() const noexcept(false);
  operator const double &() const noexcept(false);
  operator const bool &() const noexcept(false);
  operator const Json::Array &() const noexcept(false);
  operator const Json::Object &() const noexcept(false);

  // check type
  Json::Type type() const;
  bool isString() const;
  bool isArray() const;
  bool isObject() const;
  bool isNull() const;
  bool isNumber() const;
  bool isBool() const;

  bool operator==(const Json &) const;
  bool operator!=(const Json &) const;
  bool operator<(const Json &) const;
  bool operator<=(const Json &) const;
  bool operator>(const Json &) const;
  bool operator>=(const Json &) const;
  template <class X> bool operator==(const X &) const;
  template <class X> bool operator!=(const X &) const;
  template <class X> bool operator<(const X &) const;
  template <class X> bool operator<=(const X &) const;
  template <class X> bool operator>(const X &) const;
  template <class X> bool operator>=(const X &) const;

private:
  std::shared_ptr<jsonimpl::JsonValue> value_;
};

namespace jsonimpl {

class JsonValue {
public:
  virtual std::string dump() const = 0;
  virtual std::shared_ptr<JsonValue> clone() const = 0;
  virtual ~JsonValue() {}

  virtual Json::Type type() const = 0;
  virtual bool isString() const { return false; }
  virtual bool isNumber() const { return false; }
  virtual bool isArray() const { return false; }
  virtual bool isObject() const { return false; }
  virtual bool isBool() const { return false; }
  virtual bool isNull() const { return false; }

  virtual bool operator==(const JsonValue *) const = 0;
  virtual bool operator!=(const JsonValue *) const = 0;
  virtual bool operator<(const JsonValue *) const = 0;
  virtual bool operator<=(const JsonValue *) const = 0;
  virtual bool operator>(const JsonValue *) const = 0;
  virtual bool operator>=(const JsonValue *) const = 0;

  virtual Json &operator[](const std::string &) {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual Json &operator[](size_t) {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual std::string &stringValue() {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual Json::Array &arrayValue() {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual Json::Object &objectValue() {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual double &numberValue() {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
  virtual bool &boolValue() {
    MYSPACE_THROW_EX(Json::TypeError);
    throw; /*slient complier*/
  }
};
template <class X> class Value : public JsonValue {
protected:
  Value(const X &x) : value_(x) {}
  Value(X &&x) : value_(std::move(x)) {}
  bool operator==(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ == d->value_;
    return false;
  }
  bool operator!=(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ != d->value_;
    return true;
  }
  bool operator<(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ < d->value_;
    return false;
  }
  bool operator<=(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ <= d->value_;
    return false;
  }
  bool operator>(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ > d->value_;
    return false;
  }
  bool operator>=(const JsonValue *x) const override {
    if (auto d = dynamic_cast<const Value<X> *>(x))
      return value_ >= d->value_;
    return false;
  }
  X value_;
};
class JsonNull : public JsonValue {
public:
  std::string dump() const override { return "null"; }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonNull>();
  }
  Json::Type type() const override { return Json::NUL; }
  bool isNull() const override { return true; }
  bool operator==(const JsonValue *x) const override {
    return x->type() == type();
  }
  bool operator!=(const JsonValue *x) const override {
    return x->type() != type();
  }
  bool operator<(const JsonValue *) const override { return false; }
  bool operator<=(const JsonValue *x) const override {
    return x->type() == type();
  }
  bool operator>(const JsonValue *) const override { return false; }
  bool operator>=(const JsonValue *x) const override {
    return x->type() == type();
  }
};
class JsonString : public Value<std::string> {
public:
  JsonString(const std::string &x) : Value(x) {}
  JsonString(std::string &&x) : Value(std::move(x)) {}
  JsonString(const char *x) : Value(x) {}
  std::string &stringValue() override { return value_; }
  Json::Type type() const override { return Json::STR; }
  bool isString() const override { return true; }
  std::string dump() const override {
    std::stringstream ss;
    ss << '\"' << value_ << '\"';
    return ss.str();
  }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonString>(value_);
  }
};
class JsonNumber : public Value<double> {
public:
  JsonNumber(double x) : Value(x) {}
  Json::Type type() const override { return Json::NUM; }
  bool isNumber() const override { return true; }
  double &numberValue() override { return value_; }
  std::string dump() const override {
    std::stringstream ss;
    ss << value_;
    return ss.str();
  }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonNumber>(value_);
  }
};
class JsonBool : public Value<bool> {
public:
  JsonBool(bool x) : Value(x) {}
  Json::Type type() const override { return Json::BOL; }
  bool isBool() const override { return true; }
  bool &boolValue() override { return value_; }
  std::string dump() const override {
    std::stringstream ss;
    ss << std::boolalpha;
    ss << value_;
    return ss.str();
  }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonBool>(value_);
  }
};
class JsonArray : public Value<Json::Array> {
public:
  JsonArray(const Json::Array &x) : Value(x) {}
  JsonArray(Json::Array &&x) : Value(std::move(x)) {}
  Json::Type type() const override { return Json::ARR; }
  bool isArray() const override { return true; }
  Json::Array &arrayValue() override { return value_; };
  Json &operator[](size_t idx) noexcept(false) override {
    if (idx < value_.size())
      return value_[idx];
    MYSPACE_THROW_EX(Json::RangeError);
    throw; /*slient complier*/
  }
  std::string dump() const override {
    std::stringstream ss;
    ss << '[';
    bool first = true;
    for (auto &x : value_) {
      if (!first) {
        ss << ',';
      }
      first = false;
      ss << x.dump();
    }
    ss << ']';
    return ss.str();
  }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonArray>(value_);
  }
};
class JsonObject : public Value<Json::Object> {
public:
  JsonObject(const Json::Object &x) : Value(x) {}
  JsonObject(Json::Object &&x) : Value(std::move(x)) {}
  Json::Type type() const override { return Json::OBJ; }
  bool isObject() const override { return true; }
  Json::Object &objectValue() override { return value_; };
  Json &operator[](const std::string &key) override { return value_[key]; }
  std::string dump() const override {
    std::stringstream ss;
    ss << '{';
    bool first = true;
    for (auto &x : value_) {
      if (!first)
        ss << ',';
      first = false;
      ss << '\"' << x.first << "\":" << x.second.dump();
    }
    ss << '}';
    return ss.str();
  }
  std::shared_ptr<JsonValue> clone() const override {
    return new_shared<JsonObject>(value_);
  }

  friend class myspace::Json;
};

class JsonParser {
public:
  MYSPACE_EXCEPTION_DEFINE(Exception, myspace::Exception)
public:
  JsonParser(const std::string &src) : src_(src){};
  Json parse() noexcept(false) {
    try {
      return onValue(peek());
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return Json{};
  }

private:
  Json onValue(char c) noexcept(false) {
    switch (c) {
    case 't':
    case 'T':
    case 'f':
    case 'F':
      return onBool();
    case 'n':
    case 'N':
      return onNull();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '+':
    case '-':
    case '.':
      return onNumber();
    case '\"':
      return onString();
    case '[':
      return onArray();
    case '{':
      return onObject();
    default:
      MYSPACE_THROW_EX(JsonParser::Exception, "unknown value start with  ", c);
    }
    return Json{}; // not reached
  }

  Json onArray() noexcept(false) {
    try {
      assert_get('[');
      Json::Array arr;
      for (;;) {
        auto c = peek();
        if (c == ',') {
          get();
        } else if (c == ']') {
          get();
          return Json(std::move(arr));
        } else {
          arr.emplace_back(onValue(c));
        }
      }
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return Json{};
  }
  Json onObject() noexcept(false) {
    try {
      skipWhite();
      assert_get('{');
      Json::Object obj;
      std::string name;
      bool nameready = false;
      for (;;) {
        auto c = peek();
        if (c == '}') {
          get();
          return Json{obj};
        } else if (!nameready) {
          assert_peek('\"');
          name = onString();
          assert_get(':');
          nameready = true;
        } else {
          c = peek();
          if (',' == c) {
            get();
            nameready = false;
          } else {
            obj[name] = onValue(c);
          }
        }
      }
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return Json{};
  }

  std::string onString() {
    try {
      bool escape = false;
      skipWhite();
      assert_get('\"');
      std::string result;
      for (;;) {
        auto c = get(false);
        if (c == '\\') {
          if (!escape) {
            escape = true;
          } else {
            escape = false;
            result.append(1, c);
          }
        } else if ('\"' == c) {
          if (!escape)
            break;
          escape = false;
          result.append(1, c);
        } else {
          result.append(1, c);
        }
      }
      return result;
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return std::string{};
  }

  double onNumber() {
    try {
      skipWhite();
      std::stringstream result;
      for (;;) {
        auto c = peek(false);
        switch (c) {
        case ']':
        case ',':
        case '}':
          goto END_ONNUMBER;
        default:
          if (std::iscntrl(c))
            goto END_ONNUMBER;
        }
        get(false);
        result << c;
      }
    END_ONNUMBER:
      double x;
      result >> x;
      return x;
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return 0;
  }

  bool onBool() {
    try {
      skipWhite();
      std::string x;
      for (int i = 0; i < 4; ++i) {
        x.append(1, get(false));
      }
      x = Strings::tolower(x);
      if (x == "true")
        return true;
      if (x == "fals") {
        auto c = get(false);
        if (c == 'E' || c == 'e')
          return false;
      }
      MYSPACE_THROW(" expect true/false");
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return false;
  }

  Json onNull() {
    try {
      skipWhite();
      std::string x;
      for (int i = 0; i < 4; ++i) {
        x.append(1, get(false));
      }
      x = Strings::tolower(x);
      MYSPACE_THROW_IF(x != "null");
      return Json();
    } catch (...) {
      MYSPACE_DEV_RETHROW_EX(JsonParser::Exception);
    }
    return Json{};
  }

  void assert_peek(char c) {
    auto p = peek();
    MYSPACE_THROW_IF_EX(JsonParser::Exception, p != c, ", c = \'", c,
                        "\', peek = \'", p, "\'");
  }
  void assert_get(char c) {
    auto g = get();
    MYSPACE_THROW_IF_EX(JsonParser::Exception, g != c, ", c = \'", c,
                        "\', get = \'", g, "\'");
  }
  void skipWhite() {
    for (; !src_.empty(); src_.erase(0, 1)) {
      if (std::iscntrl(src_[0]) || std::isblank(src_[0]))
        continue;
      break;
    }
  }
  char get(bool skipwhite = true) {
    if (skipwhite)
      skipWhite();
    MYSPACE_THROW_IF_EX(JsonParser::Exception, src_.empty());
    MYSPACE_DEFER(src_.erase(0, 1));
    return src_[0];
  }
  char peek(bool skipwhite = true) {
    if (skipwhite)
      skipWhite();
    MYSPACE_THROW_IF_EX(JsonParser::Exception, src_.empty());
    return src_[0];
  }

  std::string src_;
}; // namespace jsonimpl

} // namespace jsonimpl

inline Json::Json(std::initializer_list<std::pair<std::string, Json>> init)
    : Json(Json::Object(init.begin(), init.end())) {}
inline Json::Json() : value_(new_shared<jsonimpl::JsonNull>()) {}
inline Json::Json(Json &&x) { swap(x); }
inline Json::Json(const Json::Object &x)
    : value_(new_shared<jsonimpl::JsonObject>(x)) {}
inline Json::Json(Json::Object &&x)
    : value_(new_shared<jsonimpl::JsonObject>(std::move(x))) {}
inline Json::Json(const Json::Array &x)
    : value_(new_shared<jsonimpl::JsonArray>(x)) {}
inline Json::Json(Json::Array &&x)
    : value_(new_shared<jsonimpl::JsonArray>(std::move(x))) {}
inline Json::Json(bool x) : value_(new_shared<jsonimpl::JsonBool>(x)) {}
inline Json::Json(const std::string &x)
    : value_(new_shared<jsonimpl::JsonString>(x)) {}
inline Json::Json(std::string &&x)
    : value_(new_shared<jsonimpl::JsonString>(std::move(x))) {}
inline Json::Json(const char *x)
    : value_(new_shared<jsonimpl::JsonString>(x)){};
inline Json::Json(double x) : value_(new_shared<jsonimpl::JsonNumber>(x)) {}
inline Json::Json(float x) : Json((double)x) {}
inline Json::Json(int32_t x) : Json((double)x) {}
inline Json::Json(uint32_t x) : Json((double)x) {}
inline Json::Json(int16_t x) : Json((double)x) {}
inline Json::Json(uint16_t x) : Json((double)x) {}
inline Json::Json(int64_t x) : Json((double)x) {}
inline Json::Json(uint64_t x) : Json((double)x) {}
inline Json::Json(const Json &x) { copy(x); }
inline void Json::swap(Json &j) {
  if (value_ != j.value_) {
    value_.swap(j.value_);
  }
}
inline void Json::copy(const Json &x) {
  if (value_ != x.value_) {
    if (!x.value_)
      value_.reset();
    else
      value_ = x.value_->clone();
  }
}
inline void Json::share(Json &x) {
  if (value_ != x.value_)
    value_ = x.value_;
}
inline Json &Json::operator=(const Json &x) {
  copy(x);
  return *this;
}
inline Json &Json::operator=(Json &&x) {
  swap(x);
  return *this;
}

inline std::string Json::dump() const {
  try {
    return value_->dump();
  } catch (...) {
    MYSPACE_DEV_EXCEPTION();
  }
  return std::string{};
}

inline std::string Json::toString() const { return dump(); }
inline std::string Json::to_json() const { return dump(); }

inline Json &Json::operator[](const std::string &key) {
  try {
    return value_->operator[](key);
  } catch (...) {
    value_ = new_shared<jsonimpl::JsonObject>(Json::Object{});
    return value_->operator[](key);
  }
}
inline const Json &Json::operator[](const std::string &key) const
    noexcept(false) {
  return value_->operator[](key);
}
template <size_t N> inline Json &Json::operator[](const char (&cstr)[N]) {
  return operator[](Strings::stripOf(std::string(cstr, N)));
}
template <size_t N>
inline const Json &Json::operator[](const char (&cstr)[N]) const
    noexcept(false) {
  return operator[](Strings::stripOf(std::string(cstr, N)));
}

inline Json &Json::operator[](size_t idx) noexcept(false) {
  return value_->operator[](idx);
}
inline const Json &Json::operator[](size_t idx) const noexcept(false) {
  return value_->operator[](idx);
}
inline std::string &Json::stringValue() noexcept(false) {
  return value_->stringValue();
}
inline Json::Array &Json::arrayValue() noexcept(false) {
  return value_->arrayValue();
}
inline Json::Object &Json::objectValue() noexcept(false) {
  return value_->objectValue();
}
inline double &Json::numberValue() noexcept(false) {
  return value_->numberValue();
}
inline bool &Json::boolValue() noexcept(false) { return value_->boolValue(); }

inline const std::string &Json::stringValue() const noexcept(false) {
  return value_->stringValue();
}
inline const Json::Array &Json::arrayValue() const noexcept(false) {
  return value_->arrayValue();
}
inline const Json::Object &Json::objectValue() const noexcept(false) {
  return value_->objectValue();
}
inline const double &Json::numberValue() const noexcept(false) {
  return value_->numberValue();
}
inline const bool &Json::boolValue() const noexcept(false) {
  return value_->boolValue();
}
inline Json::operator std::string &() noexcept(false) { return stringValue(); }
inline Json::operator double &() noexcept(false) { return numberValue(); }
inline Json::operator bool &() noexcept(false) { return boolValue(); }
inline Json::operator Json::Array &() noexcept(false) { return arrayValue(); }
inline Json::operator Json::Object &() noexcept(false) { return objectValue(); }
inline Json::operator const std::string &() const noexcept(false) {
  return stringValue();
}
inline Json::operator const double &() const noexcept(false) {
  return numberValue();
}
inline Json::operator const bool &() const noexcept(false) {
  return boolValue();
}
inline Json::operator const Json::Array &() const noexcept(false) {
  return arrayValue();
}
inline Json::operator const Json::Object &() const noexcept(false) {
  return objectValue();
}
inline Json::Type Json::type() const { return value_->type(); }
inline bool Json::isString() const { return value_->isString(); }
inline bool Json::isArray() const { return value_->isArray(); }
inline bool Json::isObject() const { return value_->isObject(); }
inline bool Json::isNumber() const { return value_->isNumber(); }
inline bool Json::isBool() const { return value_->isBool(); }
inline bool Json::isNull() const { return value_->isNull(); }

inline bool Json::operator==(const Json &x) const {
  return value_->operator==(x.value_.get());
}
inline bool Json::operator!=(const Json &x) const {
  return value_->operator!=(x.value_.get());
}
inline bool Json::operator<(const Json &x) const {
  return value_->operator<(x.value_.get());
}
inline bool Json::operator<=(const Json &x) const {
  return value_->operator<=(x.value_.get());
}
inline bool Json::operator>(const Json &x) const {
  return value_->operator>(x.value_.get());
}
inline bool Json::operator>=(const Json &x) const {
  return value_->operator>=(x.value_.get());
}
template <class X> inline bool Json::operator==(const X &x) const {
  return operator==(Json{x});
}
template <class X> inline bool Json::operator!=(const X &x) const {
  return operator!=(Json{x});
}
template <class X> inline bool Json::operator<(const X &x) const {
  return operator<(Json{x});
}
template <class X> inline bool Json::operator<=(const X &x) const {
  return operator<=(Json{x});
}
template <class X> inline bool Json::operator>(const X &x) const {
  return operator>(Json{x});
}
template <class X> inline bool Json::operator>=(const X &x) const {
  return operator>=(Json{x});
}

inline Json Json::parse(const std::string &src) noexcept(false) {
  try {
    return jsonimpl::JsonParser(src).parse();
  } catch (...) {
    MYSPACE_DEV_RETHROW_EX(Json::ParseError)
  }
  return Json{}; // not reached
}

MYSPACE_END
