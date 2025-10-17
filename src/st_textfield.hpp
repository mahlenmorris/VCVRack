#pragma once
#include "extended_text.h"
#include "NoLockQueue.h"

struct TTYQueue {
  // TODO: instead of string, string pointers, that the consumer deletes?
  SpScLockFreeQueue<std::string, 50> text_additions;
};

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
  float fontSize;
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

  // Some uses potentially won't allow the user to type text into window.
  // For a while this was set for TTY, but then I accidently dropped it and
  // it doesn't really seem to be a problem.
  bool allow_text_entry;

  // Many of the actions that require a redraw in the FramebufferWidget are only known
  // to the internals of this class.
  // So anytime something changes the visible surface, we set this to true.
  // Some higher class checks it in step() and lets FramebufferWidget know.
  // draw() clears this flag, since that's a signal that the dirt has been seen.
  bool is_dirty;

  STTextField();

  // Pulled in from oui-blendish code.
  // TODO: this argument list is stupid long.
  void myBndIconLabelCaret(NVGcontext *ctx, float x, float y, float w,
      NVGcolor color, float fontsize, int font_handle, const char *label,
      NVGcolor caretcolor, int cbegin, int cend);

  void draw(const DrawArgs& args) override;

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
  void make_additions(TTYQueue *additions);
  bool large_text_mode;
};
