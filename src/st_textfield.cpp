#include "st_textfield.hpp"
#include <ui/MenuItem.hpp>
#include <helpers.hpp>
#include <context.hpp>

struct STTextFieldCopyItem : ui::MenuItem {
  WeakPtr<STTextField> textField;
  void onAction(const ActionEvent& e) override {
    if (!textField)
      return;
    textField->copyClipboard();
    APP->event->setSelectedWidget(textField);
  }
};

struct STTextFieldCutItem : ui::MenuItem {
  WeakPtr<STTextField> textField;
  void onAction(const ActionEvent& e) override {
    if (!textField)
      return;
    textField->cutClipboard();
    APP->event->setSelectedWidget(textField);
  }
};

struct STTextFieldPasteItem : ui::MenuItem {
  WeakPtr<STTextField> textField;
  void onAction(const ActionEvent& e) override {
    if (!textField)
      return;
    textField->pasteClipboard();
    APP->event->setSelectedWidget(textField);
  }
};

struct STTextFieldSelectAllItem : ui::MenuItem {
  WeakPtr<STTextField> textField;
  void onAction(const ActionEvent& e) override {
    if (!textField)
      return;
    textField->selectAll();
    APP->event->setSelectedWidget(textField);
  }
};

STTextField::STTextField() {
  allow_text_entry = true;
  fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  fontSize = 12.0f;
  textOffset = math::Vec(3, 3);
  color = nvgRGB(0xff, 0xd7, 0x14);
  bgColor = nvgRGB(0x00, 0x00, 0x00);
  box.size.y = BND_WIDGET_HEIGHT;
  text = &placeholder;  // must be correctly set by caller!
  extended.Initialize(28, 1);
  is_dirty = true;

  // FOr TTY only.
  large_text_mode = false;
}

// I think I need this (copied from blendish.h) because it's a static function
// and not part of the API.
static void bndCaretPosition(NVGcontext *ctx, float x, float y,
    float desc, float lineHeight, const char *caret, NVGtextRow *rows,int nrows,
    int *cr, float *cx, float *cy) {
    static NVGglyphPosition glyphs[BND_MAX_GLYPHS];
    int r,nglyphs;
    for (r=0; r < nrows-1 && rows[r].end < caret; ++r);
    *cr = r;
    *cx = x;
    *cy = y-lineHeight-desc + r*lineHeight;
    if (nrows == 0) return;
    *cx = rows[r].minx;
    nglyphs = nvgTextGlyphPositions(
        ctx, x, y, rows[r].start, rows[r].end+1, glyphs, BND_MAX_GLYPHS);
    for (int i=0; i < nglyphs; ++i) {
        *cx=glyphs[i].x;
        if (glyphs[i].str == caret) break;
    }
}

void STTextField::myBndIconLabelCaret(NVGcontext *ctx,
    float x, float y, float w,
    NVGcolor color, float fontsize, int font_handle,
    const char *label, NVGcolor caretcolor, int cbegin, int cend) {
  float pleft = BND_TEXT_RADIUS;
  if (!label) return;

  if (font_handle < 0) return;

  x += pleft;
  y += BND_WIDGET_HEIGHT-BND_TEXT_PAD_DOWN;

  nvgFontFaceId(ctx, font_handle);
  nvgFontSize(ctx, fontsize);
  nvgTextAlign(ctx, NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

  w -= BND_TEXT_RADIUS + pleft;

  // Draws the selection box/caret.
  if (cend >= cbegin) {
    int box_row_begin;
    float box_x_begin, box_y_begin;
    float desc, lh;
    static NVGtextRow rows[BND_MAX_ROWS];
    // TODO: It certainly SEEMS like I should be able to replace rows with the
    // result of nvgTextBreakLines that I compute in extended_text, but in my
    // initial try, the caret did very odd things, possibly because what is
    // label+cend+1 here is the end of the text? So unlikely to be easy.
    // NOTE: may have been due the the fact that ProcessUpdatedText() was
    // guessing about font and font_size?!
    // TODO: But at the least, it seems like this could take less CPU in the UI
    // thread if we only recompute this when these arguments have changed; it
    // seems insane to recompute these on every call to drawLayer()!
    //
    // One bug in the previous impl was that we only break lines up to
    // just past cend, causing us to not know that there might be
    // text-wrapping for several characters. Thus, the cursur ends up floating
    // past the end of the line, and the cursor cannot ever get to the first
    // few characters of the following line.
    // Look forward a goodly amount, but not past the end of the string.
    // However, if we want the cursor to show up on the next line when the
    // last character is a newline, we need to go at least one char past the
    // end.
    const char * break_end = label + cend +
        std::max(1, (int) std::min(strlen(label + cend), (size_t) 50));

    int nrows = nvgTextBreakLines(
      ctx, label, break_end, w, rows, BND_MAX_ROWS);
    nvgTextMetrics(ctx, NULL, &desc, &lh);

    // Determines where to draw the highlight if any area is highlighted.
    bndCaretPosition(ctx, x, y, desc, lh, label + cbegin,
      rows, nrows, &box_row_begin, &box_x_begin, &box_y_begin);

    nvgBeginPath(ctx);
    if (cbegin == cend) {
      // Just draw a caret.
      // TODO: What color is this?
      nvgFillColor(ctx, nvgRGBf(0.337, 0.502, 0.761));
      nvgRect(ctx, box_x_begin-1, box_y_begin, 2, lh+1);
    } else {
      int box_row_end;
      float box_x_end, box_y_end;
      // Compute where the end of the highlighted region is.
      bndCaretPosition(ctx, x, y, desc, lh, label + cend,
         rows, nrows, &box_row_end, &box_x_end, &box_y_end);
      // Draw the region.
      nvgFillColor(ctx, caretcolor);
      if (box_row_begin == box_row_end) {
        nvgRect(ctx, box_x_begin - 1, box_y_begin,
                box_x_end - box_x_begin + 1, lh + 1);
      } else {
        int blk = box_row_end - box_row_begin - 1;
        nvgRect(ctx, box_x_begin - 1, box_y_begin,
                x + w - box_x_begin + 1, lh + 1);
        nvgRect(ctx, x, box_y_end, box_x_end - x + 1, lh + 1);

        if (blk)
          nvgRect(ctx, x, box_y_begin + lh, w, blk * lh + 1);
      }
    }
    nvgFill(ctx);
  }

/*
  // A silly experiment in changing text colors.
  // When it looks correct, it's not bad...
  for (int line = 0; line < std::min(nrows, 28); ++line) {
    nvgBeginPath(ctx);
    nvgFillColor(ctx, line % 2 ? SCHEME_RED : SCHEME_GREEN);
    nvgTextBox(ctx, x, y + line * lh, w, rows[line].start, rows[line].end);

  }
*/

  nvgBeginPath(ctx);
  nvgFillColor(ctx, color);
  // The original implementation left the last argument as NULL. This means
  // that any text below the bottom margin is still computed, even though it
  // is never seen, meaning that being at the top of a long text would take
  // far longer than being at the bottom.
  nvgTextBox(ctx, x, y, w, label, label + extended.VisibleTextLength());
}

void STTextField::draw(const DrawArgs& args) {
  // Code for measuring UI thread performance.
  /*
  static int count = 0;
  static long long int micros = 0;
  auto start = std::chrono::high_resolution_clock::now();
  static auto overall_start = std::chrono::high_resolution_clock::now();
  */

  // We'll clear this right away, in case some other event in the system
  // sets it while I'm drawing it.
  is_dirty = false;
 
  if (args.vg != extended.latest_nvg_context) {
    extended.setNvgContext(args.vg);
  }
  nvgScissor(args.vg, RECT_ARGS(args.clipBox));
  if (true) {
    // If the width of the box changed, we need to reindex the line structure.
    if ((box.size.x != previous_box_size_x) ||
      (previous_font_path.compare(fontPath) != 0) ||
      (previous_text.compare(*text) != 0)) {
      // textUpdated() can take an unbounded amount of time, so better to call
      // it in the UI thread than in the process() thread.
      textUpdated();
      previous_box_size_x = box.size.x;
      previous_font_path = fontPath;
      previous_text = *text;
    }

    // Text
    std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);

    if (font && font->handle >= 0) {
      bndSetFont(font->handle);

      NVGcolor highlightColor = color;
      highlightColor.a = 0.5;
      int begin = std::min(cursor, selection) - extended.CharsAbove();
      // If this is NOT the selected widget, don't show the cursor (by setting
      // 'end' to -1).
      int end = (this == APP->event->selectedWidget) ?
                std::max(cursor, selection) - extended.CharsAbove(): -1;

      if (text != nullptr) {
        myBndIconLabelCaret(args.vg,
          textOffset.x, textOffset.y, box.size.x - 2 * textOffset.x,
          color, fontSize, font->handle, text->c_str() + extended.CharsAbove(),
          highlightColor, begin, end);
      }

      bndSetFont(APP->window->uiFont->handle);
    }
  }

  OpaqueWidget::draw(args);
  nvgResetScissor(args.vg);

  // Code for measuring UI thread performance.
  /*
  auto elapsed = std::chrono::high_resolution_clock::now() - start;
  micros += std::chrono::duration_cast<std::chrono::microseconds>(
          elapsed).count();
  ++count;
  if (count >= 50) {
    auto overall_elapsed = std::chrono::high_resolution_clock::now() - overall_start;
    std::chrono::duration<float, std::milli> overall_millis = overall_elapsed;
    double overall_seconds = overall_millis.count() / 1000.0;
    WARN("** elapsed seconds = %f", overall_seconds);
    WARN("** micros/call = %lld", micros / count);
    WARN("** micros/second = %f", micros / overall_seconds);
    WARN("** calls/second = %f", count / overall_seconds);
    //WARN("*** VisibleTextLength = %d", extended.VisibleTextLength());
    micros = 0;
    count = 0;
    overall_start = std::chrono::high_resolution_clock::now();
  }
  */
}

void STTextField::onDragHover(const DragHoverEvent& e) {
  OpaqueWidget::onDragHover(e);

  if (e.origin == this) {
    int pos = getTextPosition(e.pos);
    cursor = pos;
  }
  // Draging anywhere on the widget could change the selection or cursor
  // position.
  is_dirty = true;
}

void STTextField::onButton(const ButtonEvent& e) {
  OpaqueWidget::onButton(e);

  if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
    cursor = selection = getTextPosition(e.pos);
  }

  if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT) {
    createContextMenu();
    e.consume(this);
  }
  // Clicking anywhere on the widget could change the selection or cursor
  // position.
  is_dirty = true;
}

void STTextField::onSelectText(const SelectTextEvent& e) {
  if (e.codepoint < 128) {
    std::string newText(1, (char) e.codepoint);
    if (allow_text_entry) {
      insertText(newText);
    }
  }
  e.consume(this);
}

void STTextField::onSelectKey(const SelectKeyEvent& e) {
  if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
    int original_cursor = cursor;
    // Backspace
    if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == 0) {
      if (cursor == selection) {
        cursor = std::max(cursor - 1, 0);
      }
      insertText("");
      e.consume(this);
    }
    // Ctrl+Backspace
    if (e.key == GLFW_KEY_BACKSPACE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      if (cursor == selection) {
        cursorToPrevWord();
      }
      insertText("");
      e.consume(this);
    }
    // Delete
    if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == 0) {
      if (cursor == selection) {
        cursor = std::min(cursor + 1, (int) text->size());
      }
      insertText("");
      e.consume(this);
    }
    // Ctrl+Delete
    if (e.key == GLFW_KEY_DELETE && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      if (cursor == selection) {
        cursorToNextWord();
      }
      insertText("");
      e.consume(this);
    }
    // Left
    if (e.key == GLFW_KEY_LEFT) {
      if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
        cursorToPrevWord();
      }
      else {
        cursor = std::max(cursor - 1, 0);
      }
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;
      }
      e.consume(this);
    }
    // Right
    if (e.key == GLFW_KEY_RIGHT) {
      if ((e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
        cursorToNextWord();
      }
      else {
        cursor = std::min(cursor + 1, (int) text->size());
      }
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;
      }
      e.consume(this);
    }

    // Page Up
    if (e.key == GLFW_KEY_PAGE_UP) {
      // Move up a full page length, minus one line.
      LineColumn lc = extended.GetCurrentLineColumn(cursor);
      cursor = extended.GetCursorForLineColumn(
        lc.line - extended.window_length + 1, lc.column);
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;  // Otherwise we select the line.
      }
      e.consume(this);
    }
    // Page Down
    if (e.key == GLFW_KEY_PAGE_DOWN) {
      // Move down a full page length, minus one line.
      // We do one line less that the full length so that, in the case of
      // someone just reading the whole text, the last line previously
      // visible on the screen is still visible, for continuity.
      LineColumn lc = extended.GetCurrentLineColumn(cursor);
      cursor = extended.GetCursorForLineColumn(
        lc.line + extended.window_length - 1, lc.column);
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;  // Otherwise we select the line.
      }
      e.consume(this);
    }
    // Up
    if (e.key == GLFW_KEY_UP) {
      // Move to same column, in previous line.
      LineColumn lc = extended.GetCurrentLineColumn(cursor);
      cursor = extended.GetCursorForLineColumn(lc.line - 1, lc.column);
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;  // Otherwise we select the line.
      }
      e.consume(this);
    }
    // Down
    if (e.key == GLFW_KEY_DOWN) {
      // Move to same column, in next line.
      LineColumn lc = extended.GetCurrentLineColumn(cursor);
      cursor = extended.GetCursorForLineColumn(lc.line + 1, lc.column);
      if (!(e.mods & GLFW_MOD_SHIFT)) {
        selection = cursor;  // Otherwise we select the line.
      }
      e.consume(this);
    }
    // Home
    if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == 0) {
      selection = cursor = 0;
      e.consume(this);
    }
    // Shift+Home
    if (e.key == GLFW_KEY_HOME && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
      cursor = 0;
      e.consume(this);
    }
    // End
    if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == 0) {
      selection = cursor = text->size();
      e.consume(this);
    }
    // Shift+End
    if (e.key == GLFW_KEY_END && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
      cursor = text->size();
      e.consume(this);
    }
    // Ctrl+V
    if (e.keyName == "v" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      pasteClipboard();
      e.consume(this);
    }
    // Ctrl+X
    if (e.keyName == "x" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      cutClipboard();
      e.consume(this);
    }
    // Ctrl+C
    if (e.keyName == "c" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      copyClipboard();
      e.consume(this);
    }
    // Ctrl+A
    if (e.keyName == "a" && (e.mods & RACK_MOD_MASK) == RACK_MOD_CTRL) {
      selectAll();
      e.consume(this);
    }
    // Enter
    if ((e.key == GLFW_KEY_ENTER || e.key == GLFW_KEY_KP_ENTER) && (e.mods & RACK_MOD_MASK) == 0) {
      if (allow_text_entry) {
        insertText("\n");
      }
      e.consume(this);
    }
    // Consume all printable keys unless Ctrl is held
    if ((e.mods & RACK_MOD_CTRL) == 0 && e.keyName != "") {
      // I'm not certain *how*, but this causes onSelectText() (above) to be called.
      e.consume(this);
    }

    if (cursor != original_cursor) {
      // Moved, may need to reposition window.
      extended.RepositionWindow(cursor);
    }

    // If the keyboard is being touched, odds are the text looks different now.
    is_dirty = true;

    assert(0 <= cursor);
    assert(cursor <= (int) text->size());
    assert(0 <= selection);
    assert(selection <= (int) text->size());
  }
}

int STTextField::getTextPosition(math::Vec mousePos) {
  std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);
  if (!font || !font->handle)
    return 0;

  bndSetFont(font->handle);
  int textPos = bndIconLabelTextPosition(APP->window->vg,
    textOffset.x, textOffset.y,
    box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
    -1, fontSize, text->c_str() + extended.CharsAbove(), mousePos.x, mousePos.y);
  bndSetFont(APP->window->uiFont->handle);
  return textPos + extended.CharsAbove();
}

void STTextField::textUpdated() {
  extended.ProcessUpdatedText(*text, fontPath, fontSize, box.size.x - 2 * textOffset.x);
  cursor = std::min(cursor, (int) text->size());
  selection = cursor;  // Nothing should be selected now.
  extended.RepositionWindow(cursor);
  is_dirty = true;
}

void STTextField::selectAll() {
  cursor = text->size();
  selection = 0;
}

std::string STTextField::getSelectedText() {
  int begin = std::min(cursor, selection);
  int len = std::abs(selection - cursor);
  return text->substr(begin, len);
}

bool invalidChar(char c)
{
  // \r seems to be the main thing messing me up.
  return c == '\r';
}

void STTextField::insertText(std::string new_text) {
  bool changed = false;
  if (cursor != selection) {
    // Delete selected text
    int begin = std::min(cursor, selection);
    int len = std::abs(selection - cursor);
    this->text->erase(begin, len);
    cursor = selection = begin;
    changed = true;
  }  

  // Sometimes text pasted from the outside has unprintable characters that mess
  // the display up. Let's remove them.
  new_text.erase(remove_if(new_text.begin(), new_text.end(), invalidChar),
                 new_text.end());

  if (!new_text.empty()) {
    this->text->insert(cursor, new_text);
    cursor += new_text.size();
    selection = cursor;
    changed = true;
  }
  if (changed) {
    // Since we know the text has changed, you might think we could call
    // ProcessUpdatedText() now, but doing so causes the line lengths to be
    // wrong.
    // I _think_ this might be due to unseen changes in the NVGcontext, but
    // that structure is so hard to examine that I can't be sure.
    // In any case, now we let drawLayer() see if something significant (like
    // the text) has changed, and do the ProcessUpdatedText() and
    // RepositionWindow() call then.
    //
    // Create this event so that the module using this widget can know that
    // text has changed, and add an Undo action, or compile the text,
    // or whatever it wants.
    ChangeEvent eChange;
    onChange(eChange);
  }
}

void STTextField::copyClipboard() {
  if (cursor == selection)
    return;
  glfwSetClipboardString(APP->window->win, getSelectedText().c_str());
}

void STTextField::cutClipboard() {
  copyClipboard();
  insertText("");
}

void STTextField::pasteClipboard() {
  const char* newText = glfwGetClipboardString(APP->window->win);
  if (!newText)
    return;
  if (allow_text_entry) {
    insertText(newText);
  }
}

// TODO: these should be also find words around a newline!
void STTextField::cursorToPrevWord() {
  size_t pos = text->rfind(' ', std::max(cursor - 2, 0));
  if (pos == std::string::npos)
    cursor = 0;
  else
    cursor = std::min((int) pos + 1, (int) text->size());
}

// TODO: these should be also find words around a newline!
void STTextField::cursorToNextWord() {
  size_t pos = text->find(' ', std::min(cursor + 1, (int) text->size()));
  if (pos == std::string::npos)
    pos = text->size();
  cursor = pos;
}

void STTextField::createContextMenu() {
  ui::Menu* menu = createMenu();

  if (allow_text_entry) {
    STTextFieldCutItem* cutItem = new STTextFieldCutItem;
    cutItem->text = "Cut";
    cutItem->rightText = RACK_MOD_CTRL_NAME "+X";
    cutItem->textField = this;
    menu->addChild(cutItem);
  }

  STTextFieldCopyItem* copyItem = new STTextFieldCopyItem;
  copyItem->text = "Copy";
  copyItem->rightText = RACK_MOD_CTRL_NAME "+C";
  copyItem->textField = this;
  menu->addChild(copyItem);

  if (allow_text_entry) {
    STTextFieldPasteItem* pasteItem = new STTextFieldPasteItem;
    pasteItem->text = "Paste";
    pasteItem->rightText = RACK_MOD_CTRL_NAME "+V";
    pasteItem->textField = this;
    menu->addChild(pasteItem);
  }

  STTextFieldSelectAllItem* selectAllItem = new STTextFieldSelectAllItem;
  selectAllItem->text = "Select all";
  selectAllItem->rightText = RACK_MOD_CTRL_NAME "+A";
  selectAllItem->textField = this;
  menu->addChild(selectAllItem);
}

void STTextField::make_additions(TTYQueue *additions) {
  int likely_ending_length = extended.line_map.size() +
                             additions->text_additions.size();
  bool cursor_at_end = 
    (cursor + (large_text_mode ? 1 : 0) >= (int) text->size());
  std::string item;
  while (additions->text_additions.pop(item)) {
    text->append(item);
  }
  // TODO: Shave off top lines if we're hitting the limit.
  if (likely_ending_length >= ST_MAX_ROWS) {
    int diff = extended.line_map[(int) (ST_MAX_ROWS / 10)].start_position;
    text->erase(0, extended.line_map[(int) (ST_MAX_ROWS / 10)].start_position);
    cursor = std::max(0, cursor - diff);
    selection = std::max(0, selection - diff);
  }
  if (cursor_at_end) {
    cursor = (int) text->size();
    if (large_text_mode) {
      cursor = std::max(0, cursor - 1);
    }
    // TODO: scroll?
  }
  // Reindex text.
  // TODO: maybe do this more directly?
  ChangeEvent eChange;
  onChange(eChange);
}
