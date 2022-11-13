#ifndef EXTENDED_TEXT_H
#define EXTENDED_TEXT_H

#include <algorithm>
#include <string>
#include <vector>
/*
Library that helps my subclasses of LedDisplayTextField do useful
features like being longer than the screen and support up/down keys.
*/

struct TextLine {
  int line_number;
  int start_position;
  int line_length;

  TextLine(int line, int position, int length) : line_number{line},
      start_position{position}, line_length{length} {}
  bool operator==(const TextLine a) {
    return (a.line_number == line_number) &&
           (a.start_position == start_position) &&
           (a.line_length == line_length);
  }
};

struct LineColumn {
  int line;
  int column;

  LineColumn(int line, int column) : line{line}, column{column} {}
};

struct ExtendedText {
  // Sorted searchable map from position -> line+column.
  std::vector<TextLine> line_map;
  bool initialized;

  ExtendedText() : initialized{false} {}

  void ProcessUpdatedText(const std::string &text) {
    // Clear and repopulate line_map.
    initialized = true;
    line_map.clear();
    int line_number = 0;
    // 'pos <= text.size()' is correct; the last line might be an empty line
    // directly after a \n.
    for (size_t pos = 0;  pos <= text.size(); ) {
      size_t found = text.find('\n', pos);
      if (found != std::string::npos) {
        TextLine tl(line_number, pos, found - pos);
        line_map.push_back(tl);
        pos = found + 1;
        line_number++;
      } else {
        // Add the last line.
        TextLine tl(line_number, pos, text.size() - pos);
        line_map.push_back(tl);
        break;
      }
    }
  }

  LineColumn GetCurrentLineColumn(int position) {
    int line_number;
    auto next_line = std::find_if(line_map.begin(), line_map.end(),
        [position](TextLine tl) { return position < tl.start_position; });
    if (next_line == line_map.end()) {
      // We're on the last line.
      line_number = line_map.size() - 1;
    } else {
      line_number = next_line->line_number - 1;
    }
    return LineColumn(line_number,
        position - line_map[line_number].start_position);
  }

  int GetCursorForLineColumn(int line, int column) {
    // Enforce line number limits.
    if (line < 0) {
      line = 0;
    }
    if (line >= (int) line_map.size()) {
      line = line_map.size() - 1;
    }
    TextLine tl = line_map[line];
    if (tl.line_length < column) {
      column = tl.line_length;
    }
    return tl.start_position + column;
  }
};

#endif // EXTENDED_TEXT_H
