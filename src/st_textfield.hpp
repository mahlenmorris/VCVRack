#pragma once
#include "extended_text.h"

#include <queue>
// An expansion of the VCV TextField class, but allowing for features I wish
// to add, including:
// * support for up/down keys.
// * undo/redo
// * highlighting of error line(s)
// * scrolling without a scroll bar
// And perhaps someday:
// * Text coloring
// Initial code pulled from VCV on December 8, 2022.
struct STTextField : OpaqueWidget {
	// From LedDisplayTextField.
	std::string fontPath;
  math::Vec textOffset;
  NVGcolor color;
  NVGcolor bgColor;

	std::string* text = nullptr;  // Pointer to the text in the owning module.
	std::string placeholder;
	/** The index of the text cursor */
	int cursor = 0;
	/** The index of the other end of the selection.
	If nothing is selected, this is equal to `cursor`.
	*/
	int selection = 0;

	// Tracks lines positions and lengths.
	ExtendedText extended;

  // Tracking if width of window has changed.
	float previous_box_size_x = 0.0f;
	// Tracking if font of window has changed.
	std::string previous_font_path;
	// TODO: get rid of this?
	std::string previous_text;

	// Some uses (e.g., TTY) don't allow the user to type text into window.
	bool allow_text_entry;

	STTextField();

  // Pulled in from oui-blendish code.
	// TODO: this argument list is stupid long.
	void myBndIconLabelCaret(NVGcontext *ctx, float x, float y, float w,
	    NVGcolor color, float fontsize, int font_handle, const char *label,
	    NVGcolor caretcolor, int cbegin, int cend);

	void drawLayer(const DrawArgs& args, int layer) override;

	void onDragHover(const DragHoverEvent& e) override;
	void onButton(const ButtonEvent& e) override;
	void onSelectText(const SelectTextEvent& e) override;
	void onSelectKey(const SelectKeyEvent& e) override;
	virtual int getTextPosition(math::Vec mousePos);

	// Replaces the entire text. There are cases when this is needed, like
	// undo/redo, loading. But we don't actually need to _set_ the text,
	// since the module can do that. But we do need to tell the STTextField
	// that the underlying text has chanhged (like to update internal data
  // structures).
	void textUpdated();
	void selectAll();
	std::string getSelectedText();
	/** Inserts text at the cursor, replacing the selection if necessary */
	void insertText(std::string text);
	void copyClipboard();
	void cutClipboard();
	void pasteClipboard();
	void cursorToPrevWord();
	void cursorToNextWord();
	void createContextMenu();

	// For TTY. Adds the following lines to text, removes top lines if too long,
	// moves cursor if already at end.
	void make_additions(std::queue<std::string> *additions);

};
