#ifndef EXTENDED_TEXT_H
#define EXTENDED_TEXT_H

#include <algorithm>
#include <string>
#include <vector>

#include "plugin.hpp"

/*
Library that helps my subclasses of LedDisplayTextField do useful
features like being longer than the screen and support up/down keys.
*/

// For Fermata and TTY, set up map of visible line counts to font size
// and Y offset.
// Each entry is:
//   [line_count, font size, Y_offset]
extern int LARGER_TEXT_INFO[13][3];

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
  bool operator==(const LineColumn a) {
    return (a.line == line) && (a.column == column);
  }
};

// Maximum physical rows we can usefully have in the document. At narrow widths,
// the number of physical rows can be quite large.
// TODO: artificially lower this and ensure this doesn't crash Rack or break
// too badly.
#define ST_MAX_ROWS 1000

struct ExtendedText {
  // Sorted searchable map from position -> line+column.
  std::vector<TextLine> line_map;
  // Related data structure used to draw the actual lines.
  int total_physical_row_count;
  NVGtextRow rows[ST_MAX_ROWS];
  // Number of lines in the visible area.
  int window_length;
  // Number of lines away from the edge before we start scrolling.
  // Zero may not be a value that I can get to work.
  int buffer_length;
  // Track how many lines are above the visible window.
  // -1 signals that we are ready to completely reassign the window.
  // For example, at startup.
  int lines_above = -1;
  // Needed (from call to drawLayer()) to call nvgTextBreakLines().
  NVGcontext* latest_nvg_context = nullptr;

  void Initialize(int length, int buffer);

  void setNvgContext(NVGcontext* context);

  // The position that the window shows is always relative to the text *in
  // the window*, and not the whole text. So there are places we need to
  // surface that.
  int CharsAbove();

  int VisibleTextLength();

  // With the addition of 'rows' and us maintaining knowledge of the physical
  // lines, this should be called whenever:
  // * the text is updated
  // * The window width is changed
  // * The font is changed
  // * when started, and before we display anything.
  void ProcessUpdatedText(const std::string &text, const std::string &font_path,
     float width);

  // Given a cursor position, where are we?
  LineColumn GetCurrentLineColumn(int position);
  // Before calling this, line_map *must* be accurate to the current state of
  // 'text'.
  // Updates lines_above.
  void RepositionWindow(int current_full_position);
  int GetCursorForLineColumn(int line, int column);
};

#endif // EXTENDED_TEXT_H
