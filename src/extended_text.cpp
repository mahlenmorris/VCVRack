#include <algorithm>
#include <string>
#include <vector>

#include "extended_text.h"

int LARGER_TEXT_INFO[13][3] = {
  {28, 12, 3},
  {24, 14, 4},
  {20, 17, 4},
  {17, 20, 6},
  {14, 24, 10},
  {11, 31, 14},
  {8, 42, 20},
  {6, 56, 28},
  {5, 67, 35},
  {4, 84, 45},
  {3, 112, 61},
  {2, 168, 94},
  {1, 400, 240}
};

void ExtendedText::Initialize(int length, int buffer) {
  window_length = length;
  buffer_length = buffer;
}

void ExtendedText::setNvgContext(NVGcontext* context) {
  latest_nvg_context = context;
}

// The position that the window shows is always relative to the text *in
// the window*, and not the whole text. So there are places we need to
// surface that.
int ExtendedText::CharsAbove() {
  if (line_map.empty()) return 0;

  if (lines_above < 0) {
    return 0;
  } else {
    if (lines_above < (int) line_map.size()) {
      return line_map[lines_above].start_position;
    } else {
      // Not sure what's happening at this point.
      return 0;
    }
  }
}

int ExtendedText::VisibleTextLength() {
  if (line_map.empty()) return 0;

  // Find first line not visible on screen, or end of text.
  int line = lines_above + window_length;
  if (line >= total_physical_row_count) {
    return line_map.back().start_position + line_map.back().line_length -
      CharsAbove();
  } else {
    return line_map[line].start_position + line_map[line].line_length -
      CharsAbove();
  }
}

// With the addition of 'rows' and us maintaining knowledge of the physical
// lines, this should be called whenever:
// * the text is updated
// * The window width is changed
// * The font is changed
// * when started, and before we display anything.
void ExtendedText::ProcessUpdatedText(const std::string &text,
   const std::string &font_path, float font_size, float width) {
  if (latest_nvg_context == nullptr) {
    return;
    // TODO: This is NOT correct. Should really set a dirty bit indicating
    // that we should recompute this once we get context.
  }
  // Clear and repopulate line_map.
  line_map.clear();

  // The "-2*BND_TEXT_RADIUS" I am just copying from original TextField.
  width -= 2 * BND_TEXT_RADIUS;
  std::shared_ptr<window::Font> font = APP->window->loadFont(font_path);
  if (font && font->handle >= 0) {
    nvgFontFaceId(latest_nvg_context, font->handle);
    nvgFontSize(latest_nvg_context, font_size);  // Font size we currently use, sometimes adjusted in menu.
    nvgTextAlign(latest_nvg_context, NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

    total_physical_row_count = nvgTextBreakLines(
      latest_nvg_context, text.c_str(), NULL, width,
      rows, ST_MAX_ROWS);

    // Convert to our line map.
    for (int row = 0; row < total_physical_row_count; row++) {
      TextLine tl(row, rows[row].start - text.c_str(),
        rows[row].end - rows[row].start);
      line_map.push_back(tl);
    }
    // If the last character of the text is a newline, there's another line
    // to add.
    // And selection can crash Rack if there are no lines.
    if ((text.size() == 0) || (text.back() == '\n')) {
      TextLine tl(line_map.size(), text.size(), 0);
      line_map.push_back(tl);
    }
  }
}

// Given a cursor position, where are we?
LineColumn ExtendedText::GetCurrentLineColumn(int position) {
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

// Before calling this, line_map *must* be accurate to the current state of
// 'text'.
// Updates lines_above.
void ExtendedText::RepositionWindow(int current_full_position) {
  if (line_map.empty()) return;
  // Which line are we currently on?
  LineColumn lc = GetCurrentLineColumn(current_full_position);
  // First, figure out what we want.
  // For this to really work, we need to know what the top line
  // of the existing window text is. That's what lines_above is for.
  // lines_above also gives us a sense of which direction user came from to
  // get to current_position.
  int top_line;
  if (lines_above < 0) {
    // I now think this only happens when program is first loaded,
    // and we are already looking at the top.
    top_line = 0;
  } else {
    if (lc.line >= lines_above + window_length - buffer_length) {
      // We are below visible text, move down.
      top_line = lc.line - window_length + buffer_length + 1;
    } else if (lc.line <= lines_above + buffer_length) {
      // We are getting close to the top, move so position is on the top line.
      top_line = std::max(0, lc.line - buffer_length);
    } else {
      // Position is currently visible, do not change the visible text.
      return;
    }
  }
  lines_above = top_line;
  return;
}

int ExtendedText::GetCursorForLineColumn(int line, int column) {
  if (line_map.empty()) return 0;
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
