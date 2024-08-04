#pragma once

namespace json::query {

class PathCompiler {
 public:
  static constexpr char kDocContext = '$';
  static constexpr char kEvalContext = '@';
  static constexpr char kOpenBracket = '[';
  static constexpr char kCloseBracket = ']';
  static constexpr char kOpenParenthesis = '(';
  static constexpr char kCloseParenthesis = ')';
  static constexpr char kOpenBrace = '{';
  static constexpr char kCloseBrace = '}';

  static constexpr char kWildcard = '*';
  static constexpr char kPeriod = '.';
  static constexpr char kSpace = ' ';
  static constexpr char kTAB = '\t';
  static constexpr char kCR = '\r';
  static constexpr char kLF = '\n';
  static constexpr char kFilter = '?';
  static constexpr char kComma = ',';
  static constexpr char kSplit = ':';
  static constexpr char kMinus = '-';
  static constexpr char kSingleQuote = '\'';
  static constexpr char kDoubleQuote = '"';
};

}  // namespace json::query