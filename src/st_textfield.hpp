#pragma once
#include "plugin.hpp"
#include "extended_text.h"

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

	/** For Tab and Shift-Tab focusing.
	*/
	Widget* prevField = NULL;
	Widget* nextField = NULL;

	STTextField();
	void drawLayer(const DrawArgs& args, int layer) override;

	void onDragHover(const DragHoverEvent& e) override;
	void onButton(const ButtonEvent& e) override;
	void onSelectText(const SelectTextEvent& e) override;
	void onSelectKey(const SelectKeyEvent& e) override;
	virtual int getTextPosition(math::Vec mousePos);

	std::string getText();  // TODO: eliminate!
	/** Replaces the entire text */
	void setText(std::string text);  // TODO: eliminate!
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
};
