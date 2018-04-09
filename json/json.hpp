

#pragma once

#include "myspace/_/include.hpp"
#include "myspace/code/reuse.hpp"

MYSPACE_BEGIN

namespace jsonimpl {
class JsonValue;
}

class Json {
public:
  enum Type { STR, OBJ, NUM, NUL, ARR, BOL };

  typedef map<string, Json> Object;
  typedef deque<Json> Array;

public:
  static Json parse(const string &src);
  static Json parse(const string &src, string &err);
  static Json parse(const char *src);
  static Json parse(const char *src, string &err);

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

  // Implicit constructor: vector-like objects (std::list, std::vector,
  // std::set, etc)
  template <class V, typename std::enable_if<
                         std::is_constructible<
                             Json, decltype(*std::declval<V>().begin())>::value,
                         int>::type = 0>
  Json(const V &v) : Json(Array(v.begin(), v.end())) {}
  Json(initializer_list<pair<string, Json>> init);
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
  Json(const string &);
  Json(string &&);
  Json(const char *);
  Json(Json &&);
  Json(const Json &);
  void swap(Json &);
  void copy(const Json &);
  void share(Json &);
  Json &operator=(const Json &);
  Json &operator=(Json &&);

  string dump() const;

  // access
  const Json &operator[](const string &) const;
  const Json &operator[](size_t) const;
  const string &stringValue() const;
  const Array &arrayValue() const;
  const Object &objectValue() const;
  double numberValue() const;
  bool boolValue() const;

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

private:
  shared_ptr<jsonimpl::JsonValue> value_;
};

namespace jsonimpl {
class JsonValue {
  // friend class Json;
public:
  virtual string dump() const = 0;
  virtual shared_ptr<JsonValue> clone() const = 0;
  virtual ~JsonValue() {}
  virtual const Json &operator[](const string &) const {
    static Json j;
    return j;
  }
  virtual const Json &operator[](size_t) const {
    static Json j;
    return j;
  }

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

  virtual const string &stringValue() const {
    static string str;
    return str;
  }
  virtual const Json::Array &arrayValue() const {
    static Json::Array arr;
    return arr;
  }
  virtual const Json::Object &objectValue() const {
    static Json::Object obj;
    return obj;
  }
  virtual double numberValue() const { return 0; }
  virtual bool boolValue() const { return false; }
};
template <class X> class Value : public JsonValue {
protected:
  Value(const X &x) : value_(x) {}
  Value(X &&x) : value_(move(x)) {}
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
  string dump() const override { return "null"; }
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonNull>();
  }
  Json::Type type() const override { return Json::NUL; }
  bool isNull() const override { return true; }
  bool operator==(const JsonValue *x) const override {
    if (dynamic_cast<const JsonNull *>(x))
      return true;
    return false;
  }
  bool operator!=(const JsonValue *x) const override {
    if (dynamic_cast<const JsonNull *>(x))
      return false;
    return true;
  }
  bool operator<(const JsonValue *) const override { return false; }
  bool operator<=(const JsonValue *x) const override {
    if (dynamic_cast<const JsonNull *>(x))
      return true;
    return false;
  }
  bool operator>(const JsonValue *) const override { return false; }
  bool operator>=(const JsonValue *x) const override {
    if (dynamic_cast<const JsonNull *>(x))
      return true;
    return false;
  }
};
class JsonString : public Value<string> {
public:
  JsonString(const string &x) : Value(x) {}
  JsonString(string &&x) : Value(move(x)) {}
  JsonString(const char *x) : Value(x) {}
  const string &stringValue() const override { return value_; }
  Json::Type type() const override { return Json::STR; }
  bool isString() const override { return true; }
  string dump() const override {
    stringstream ss;
    ss << '\"' << value_ << '\"';
    return ss.str();
  }
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonString>(value_);
  }
};
class JsonNumber : public Value<double> {
public:
  JsonNumber(double x) : Value(x) {}
  Json::Type type() const override { return Json::NUM; }
  bool isNumber() const override { return true; }
  double numberValue() const override { return value_; }
  string dump() const override {
    stringstream ss;
    ss << value_;
    return ss.str();
  }
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonNumber>(value_);
  }
};
class JsonBool : public Value<bool> {
public:
  JsonBool(bool x) : Value(x) {}
  Json::Type type() const override { return Json::BOL; }
  bool isBool() const override { return true; }
  bool boolValue() const override { return value_; }
  string dump() const override {
    stringstream ss;
    ss << boolalpha;
    ss << value_;
    return ss.str();
  }
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonBool>(value_);
  }
};
class JsonArray : public Value<Json::Array> {
public:
  JsonArray(const Json::Array &x) : Value(x) {}
  JsonArray(Json::Array &&x) : Value(move(x)) {}
  Json::Type type() const override { return Json::ARR; }
  bool isArray() const override { return true; }
  const Json::Array &arrayValue() const override { return value_; };
  const Json &operator[](size_t idx) const override {
    if (idx < value_.size())
      return value_[idx];
    return JsonValue::operator[](idx);
  }
  string dump() const override {
    stringstream ss;
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
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonArray>(value_);
  }
};
class JsonObject : public Value<Json::Object> {
  friend class Json;

public:
  JsonObject(const Json::Object &x) : Value(x) {}
  JsonObject(Json::Object &&x) : Value(move(x)) {}
  Json::Type type() const override { return Json::OBJ; }
  bool isObject() const override { return true; }
  const Json::Object &objectValue() const override { return value_; };
  const Json &operator[](const string &key) const override {
    auto itr = value_.find(key);
    if (itr != value_.end())
      return itr->second;
    return JsonValue::operator[](key);
  }
  string dump() const override {
    stringstream ss;
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
  shared_ptr<JsonValue> clone() const override {
    return make_shared<JsonObject>(value_);
  }
};

} // namespace jsonimpl

inline Json::Json(initializer_list<pair<string, Json>> init)
    : Json(Json::Object(init.begin(), init.end())) {}
inline Json::Json() : value_(make_shared<jsonimpl::JsonNull>()) {}
inline Json::Json(Json &&x) { swap(x); }
inline Json::Json(const Json::Object &x)
    : value_(make_shared<jsonimpl::JsonObject>(x)) {}
inline Json::Json(Json::Object &&x)
    : value_(make_shared<jsonimpl::JsonObject>(move(x))) {}
inline Json::Json(const Json::Array &x)
    : value_(make_shared<jsonimpl::JsonArray>(x)) {}
inline Json::Json(Json::Array &&x)
    : value_(make_shared<jsonimpl::JsonArray>(move(x))) {}

inline Json::Json(bool x) : value_(make_shared<jsonimpl::JsonBool>(x)) {}
inline Json::Json(const string &x)
    : value_(make_shared<jsonimpl::JsonString>(x)) {}
inline Json::Json(string &&x)
    : value_(make_shared<jsonimpl::JsonString>(move(x))) {}
inline Json::Json(const char *x)
    : value_(make_shared<jsonimpl::JsonString>(x)){};
inline Json::Json(double x) : value_(make_shared<jsonimpl::JsonNumber>(x)) {}
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
Json &Json::operator=(const Json &x) {
  copy(x);
  return *this;
}
Json &Json::operator=(Json &&x) {
  swap(x);
  return *this;
}

inline string Json::dump() const { return move(value_->dump()); }

namespace jsonimpl {
class JsonParser {
public:
  JsonParser(const char *src) : src_(src){};
  Json parse() {
    skipWhite();
    try {
      auto c = peek();
      switch (c) {
      case '+':
      case '-':
      case '.':
      MYSPACE_CASE_0_9:
        return onNumber();
        break;
      case 'n':
      case 'N':
        return onNull();
      case '\"':
        return onString();
      case 't':
      case 'T':
      case 'F':
      case 'f':
        return onBool();
      case '[':
        return onArray();
      case '{':
        return onObject();
      }
    } catch (...) {
      MYSPACE_DEV(Exception::dump());
    }
    return Json();
  }

private:
  Json onArray() {
    try {
      assert_get('[');
      Json::Array arr;
      for (;;) {
        auto c = peek();
        switch (c) {
        case ',':
          get();
          break;
        case ']':
          get();
          return Json(move(arr));
        case '\"':
          arr.emplace_back(move(onString()));
          break;
        case '[':
          arr.emplace_back(move(onArray()));
          break;
        case 't':
        case 'T':
        case 'f':
        case 'F':
          arr.emplace_back(move(onBool()));
          break;
        case 'N':
        case 'n':
          arr.emplace_back(move(onNull()));
          break;
        case '{':
          arr.emplace_back(move(onObject()));
          break;
        case '+':
        case '-':
        case '.':
        MYSPACE_CASE_0_9:
          arr.emplace_back(move(onNumber()));
          break;
        default:
          MYSPACE_THROW(" unknown char ", c);
        }
      }
    } catch (...) {
      MYSPACE_THROW("decode array failed");
    }
    return Json();
  }
  Json onObject() {
    try {
      skipWhite();
      assert_get('{');
      Json::Object obj;
      string name;
      bool nameready = false;
      for (;;) {
        auto c = peek();
        MYSPACE_DEV("c = \'%s\', name ready = %s", c, nameready);
        if (c == '}') {
          get();
          return Json(move(obj));
        } else if (!nameready) {
          // MYSPACE_DEV("??");
          assert_peek('\"');
          // MYSPACE_DEV("!!");
          name = onString();
          assert_get(':');
          nameready = true;
        } else {
          c = peek();
          switch (c) {
          case ',':
            get();
            nameready = false;
            break;
          case '\"': {
            Json j(onString());
            obj[name].swap(j);
            break;
          }
          case '[': {
            Json j(onArray());
            obj[name].swap(j);
            break;
          }
          case '{': {
            Json j(onObject());
            obj[name].swap(j);
            break;
          }
          case 't':
          case 'T':
          case 'f':
          case 'F': {
            Json j(onBool());
            obj[name].swap(j);
            break;
          }
          case 'n':
          case 'N': {
            Json j(onNull());
            obj[name].swap(j);
            break;
          }
          case '.':
          case '-':
          case '+':
          MYSPACE_CASE_0_9 : {
            Json j(onNumber());
            obj[name].swap(j);
            break;
          }
          default:
            MYSPACE_THROW("unexpected char : ", c);
          }
        }
      }
    } catch (...) {
      MYSPACE_THROW("decode object failed");
    }
    return Json();
  }

  string onString() {
    try {
      bool escape = false;
      skipWhite();
      assert_get('\"');
      string result;
      for (;;) {
        auto c = get(false);
        switch (c) {
        case '\\':
          if (!escape) {
            escape = true;
          } else {
            escape = false;
            result.append(1, c);
          }
          break;
        case '\"':
          if (escape) {
            escape = false;
            result.append(1, c);
            break;
          }
          goto END_ONSTRING;
        default:
          result.append(1, c);
        }
      }
    END_ONSTRING:
      return move(result);
    } catch (...) {
      MYSPACE_THROW("decode string failed");
    }
    return "";
  }

  double onNumber() {
    try {
      skipWhite();
      stringstream result;
      for (;;) {
        auto c = peek(false);
        MYSPACE_DEV(" c = %s", c);
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
      MYSPACE_THROW("decode number failed");
    }
    return 0;
  }

  bool onBool() {
    try {
      skipWhite();
      string x;
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
      MYSPACE_THROW("decode bool failed");
    }
    return false;
  }

  Json onNull() {
    try {
      skipWhite();
      string x;
      for (int i = 0; i < 4; ++i) {
        x.append(1, get(false));
      }
      x = Strings::tolower(x);
      MYSPACE_IF_THROW(x != "null");
      return Json();
    } catch (...) {
      MYSPACE_THROW("decode null failed");
    }
    return Json();
  }

  void assert_peek(char c) {
    auto p = peek();
    MYSPACE_IF_THROW(p != c, ", c = \'", c, "\', peek = \'", p, "\'");
  }
  void assert_get(char c) {
    auto g = get();
    MYSPACE_IF_THROW(g != c, ", c = \'", c, "\', get = \'", g, "\'");
  }
  void skipWhite() {
    for (; *src_; ++src_) {
      if (std::iscntrl(*src_) || std::isblank(*src_))
        continue;
      break;
    }
  }
  char get(bool skipwhite = true) {
    // MYSPACE_DEV("get");
    if (skipwhite)
      skipWhite();
    MYSPACE_IF_THROW(!*src_);
    return *(src_++);
  }
  char peek(bool skipwhite = true) {
    // MYSPACE_DEV("peek");
    if (skipwhite)
      skipWhite();
    MYSPACE_IF_THROW(!*src_);
    return *(src_);
  }

  const char *src_ = nullptr;
};
} // namespace jsonimpl

inline Json Json::parse(const string &src) {
  return move(jsonimpl::JsonParser(src.c_str()).parse());
}
inline Json Json::parse(const string &src, string &) {
  return move(jsonimpl::JsonParser(src.c_str()).parse());
}
inline Json Json::parse(const char *src) {
  return move(jsonimpl::JsonParser(src).parse());
}
inline Json Json::parse(const char *src, string &) {
  return move(jsonimpl::JsonParser(src).parse());
}
inline const Json &Json::operator[](const string &key) const {
  return value_->operator[](key);
}
inline const Json &Json::operator[](size_t idx) const {
  return value_->operator[](idx);
}
inline const string &Json::stringValue() const { return value_->stringValue(); }
inline const Json::Array &Json::arrayValue() const {
  return value_->arrayValue();
}
inline const Json::Object &Json::objectValue() const {
  return value_->objectValue();
}
inline double Json::numberValue() const { return value_->numberValue(); }
inline bool Json::boolValue() const { return value_->boolValue(); }

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
MYSPACE_END
