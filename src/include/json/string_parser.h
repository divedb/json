#pragma once

#include "json/json_value.h"
#include "json/pipe.h"

namespace json {

// TODO(gc): do we need static storage here?
inline constexpr auto quotation_mark_pipe = PipeOne(is_byte<'"'>);

// string = quotation-mark *char quotation-mark
// char   =   unescaped
//          | escape (
//                      "       quotation mark      U+0022
//                      \       reverse solidus     U+005C
//                      /       solidus             U+002F
//                      b       backspace           U+0008
//                      f       form feed           U+000C
//                      n       line feed           U+000A
//                      r       carriage return     U+000D
//                      t       tab                 U+0009
//                      uXXXX                       U+XXXX
//                   )
// escape           = %x5C
// question-mark    = %x22
// unescaped        s= %x20-21 | %x23-5B | %x5D-10FFFF
template <typename InputIt>
inline JsonValue parse_string(ParserState<InputIt>& state) {
  JsonValue json_value;

  if (state | quotation_mark_pipe; !state.is_ok()) {
    return json_value;
  }

  while (state.has_next()) {
    state.succeed_pipes = 0;

    if (state | escape_pipe; state.is_ok()) {
      continue;
    }

    if (state.succeed_pipes > 0) {
      break;
    }

    state.status = Status::kSucceed;

    if (state | quotation_mark_pipe; state.is_ok()) {
      json_value = JsonValue(state.buffer());
      break;
    }

    state | sink_pipe;
  }

  return json_value;
}

}  // namespace json
