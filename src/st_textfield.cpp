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
  fontPath = asset::system("res/fonts/ShareTechMono-Regular.ttf");
  textOffset = math::Vec(3, 3);
  color = nvgRGB(0xff, 0xd7, 0x14);
  bgColor = nvgRGB(0x00, 0x00, 0x00);
	box.size.y = BND_WIDGET_HEIGHT;
  text = &placeholder;  // must be correctly set by caller!
	extended.Initialize(28, 1);
}

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
	  float x, float y, float w, float h,
    int iconid, NVGcolor color, float fontsize, int font_handle,
		const char *label, NVGcolor caretcolor, int cbegin, int cend) {
  float pleft = BND_TEXT_RADIUS;
  if (!label) return;
  if (iconid >= 0) {
      bndIcon(ctx, x + 4, y + 2, iconid);
      pleft += BND_ICON_SHEET_RES;
  }

  if (font_handle < 0) return;

  x += pleft;
  y += BND_WIDGET_HEIGHT-BND_TEXT_PAD_DOWN;

  nvgFontFaceId(ctx, font_handle);
  nvgFontSize(ctx, fontsize);
  nvgTextAlign(ctx, NVG_ALIGN_LEFT|NVG_ALIGN_BASELINE);

  w -= BND_TEXT_RADIUS + pleft;

  // Draws the selection box/caret.
  if (cend >= cbegin) {
    int c0r, c1r;  // TODO: fix thes ghastly variable names.
    float c0x, c0y, c1x, c1y;
    float desc, lh;
    static NVGtextRow rows[BND_MAX_ROWS];
    int nrows = nvgTextBreakLines(
      ctx, label, label + cend + 1, w, rows, BND_MAX_ROWS);
    nvgTextMetrics(ctx, NULL, &desc, &lh);

    bndCaretPosition(ctx, x, y, desc, lh, label + cbegin,
      rows, nrows, &c0r, &c0x, &c0y);
    bndCaretPosition(ctx, x, y, desc, lh, label + cend,
      rows, nrows, &c1r, &c1x, &c1y);

    nvgBeginPath(ctx);
    if (cbegin == cend) {
		  // Just draw a caret.
			// TODO: What color is this?
      nvgFillColor(ctx, nvgRGBf(0.337, 0.502, 0.761));
      nvgRect(ctx, c0x-1, c0y, 2, lh+1);
    } else {
			// Draw the region.
      nvgFillColor(ctx, caretcolor);
      if (c0r == c1r) {
        nvgRect(ctx, c0x - 1, c0y, c1x - c0x + 1, lh + 1);
      } else {
        int blk = c1r - c0r - 1;
        nvgRect(ctx, c0x - 1, c0y, x + w - c0x + 1, lh + 1);
        nvgRect(ctx, x, c1y, c1x - x + 1, lh + 1);

        if (blk)
          nvgRect(ctx, x, c0y + lh, w, blk * lh + 1);
      }
    }
    nvgFill(ctx);
  }

  nvgBeginPath(ctx);
  nvgFillColor(ctx, color);
  nvgTextBox(ctx, x, y, w, label, NULL);
}

void STTextField::drawLayer(const DrawArgs& args, int layer) {
	nvgScissor(args.vg, RECT_ARGS(args.clipBox));
	if (layer == 1) {
		// Text
		std::shared_ptr<window::Font> font = APP->window->loadFont(fontPath);

		if (font && font->handle >= 0) {
			bndSetFont(font->handle);

			NVGcolor highlightColor = color;
			highlightColor.a = 0.5;
			int begin = std::min(cursor, selection) - extended.CharsAbove();
			int end = (this == APP->event->selectedWidget) ?
			          std::max(cursor, selection) - extended.CharsAbove(): -1;

      if (text != nullptr) {
  			myBndIconLabelCaret(args.vg,
  				textOffset.x, textOffset.y,
  				box.size.x - 2 * textOffset.x, box.size.y - 2 * textOffset.y,
  				-1, color, 12, font->handle, text->c_str() + extended.CharsAbove(),
				  highlightColor, begin, end);
      }

			bndSetFont(APP->window->uiFont->handle);
		}
	}

	Widget::drawLayer(args, layer);
	nvgResetScissor(args.vg);
}

void STTextField::onDragHover(const DragHoverEvent& e) {
	OpaqueWidget::onDragHover(e);

	if (e.origin == this) {
		int pos = getTextPosition(e.pos);
		cursor = pos;
	}
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
}

void STTextField::onSelectText(const SelectTextEvent& e) {
	if (e.codepoint < 128) {
		std::string newText(1, (char) e.codepoint);
		insertText(newText);
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
  		insertText("\n");
			e.consume(this);
		}
		// Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == 0) {
			if (nextField)
				APP->event->setSelectedWidget(nextField);
			e.consume(this);
		}
		// Shift+Tab
		if (e.key == GLFW_KEY_TAB && (e.mods & RACK_MOD_MASK) == GLFW_MOD_SHIFT) {
			if (prevField)
				APP->event->setSelectedWidget(prevField);
			e.consume(this);
		}
		// Consume all printable keys unless Ctrl is held
		if ((e.mods & RACK_MOD_CTRL) == 0 && e.keyName != "") {
			e.consume(this);
		}

		if (cursor != original_cursor) {
			// Moved, may need to reposition window.
			extended.RepositionWindow(cursor);
		}

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
		-1, 12, text->c_str() + extended.CharsAbove(), mousePos.x, mousePos.y);
	bndSetFont(APP->window->uiFont->handle);
	return textPos + extended.CharsAbove();
}

void STTextField::textUpdated() {
  extended.ProcessUpdatedText(*text);
  // TODO: this is not _exactly_ what we want. Maybe save cursor+selection
  // in undo/redo?
  cursor = std::min(cursor, (int) text->size());
	selection = cursor;  // Nothing should be selected now.
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
		// Update the line map.
		extended.ProcessUpdatedText(*text);
		extended.RepositionWindow(cursor);
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
	insertText(newText);
}

void STTextField::cursorToPrevWord() {
	size_t pos = text->rfind(' ', std::max(cursor - 2, 0));
	if (pos == std::string::npos)
		cursor = 0;
	else
		cursor = std::min((int) pos + 1, (int) text->size());
}

void STTextField::cursorToNextWord() {
	size_t pos = text->find(' ', std::min(cursor + 1, (int) text->size()));
	if (pos == std::string::npos)
		pos = text->size();
	cursor = pos;
}

void STTextField::createContextMenu() {
	ui::Menu* menu = createMenu();

	STTextFieldCutItem* cutItem = new STTextFieldCutItem;
	cutItem->text = "Cut";
	cutItem->rightText = RACK_MOD_CTRL_NAME "+X";
	cutItem->textField = this;
	menu->addChild(cutItem);

	STTextFieldCopyItem* copyItem = new STTextFieldCopyItem;
	copyItem->text = "Copy";
	copyItem->rightText = RACK_MOD_CTRL_NAME "+C";
	copyItem->textField = this;
	menu->addChild(copyItem);

	STTextFieldPasteItem* pasteItem = new STTextFieldPasteItem;
	pasteItem->text = "Paste";
	pasteItem->rightText = RACK_MOD_CTRL_NAME "+V";
	pasteItem->textField = this;
	menu->addChild(pasteItem);

	STTextFieldSelectAllItem* selectAllItem = new STTextFieldSelectAllItem;
	selectAllItem->text = "Select all";
	selectAllItem->rightText = RACK_MOD_CTRL_NAME "+A";
	selectAllItem->textField = this;
	menu->addChild(selectAllItem);
}
