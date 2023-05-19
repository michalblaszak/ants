#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/WTextArea.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>
#include <Wt/Utils.h>
#include <Wt/WFont.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <algorithm>

enum class EBackgroundStyle : int {
    NORMAL   = 1 << 0, 
    KEYWORD  = 1 << 1,
    SELECTED = 1 << 2
};
enum class EForegroundStyle : int {
    NORMAL  = 1 << 0,
    KEYWORD = 1 << 1
};

extern Wt::WPen ForegroundNormal;
extern Wt::WBrush BackgroundNormal;
extern Wt::WPen BackgroundBorderNormal;

extern Wt::WBrush BackgroundSelected;
extern Wt::WPen BackgroundBorderSelected;

extern Wt::WPen ForegroundKeyword;
extern Wt::WBrush BackgroundKeyword;
extern Wt::WPen BackgroundBorderKeyword;

extern Wt::WPen CurrentLineBorder;
extern Wt::WBrush CurrentLineBackground;

extern Wt::WPen RowNumbersCurrentLine;
extern Wt::WPen RowNumbersNormalLine;

class CodeEditor;

class Character {
public:
    Character();
    Character(std::string utf8, EForegroundStyle foreStyle, EBackgroundStyle backStyle);
    Character(const Character& other);

    Character& operator=(const Character& other);

    bool operator==(const Character& other);

    bool isSameStyle(const Character& other) const;

    const Wt::WPen& getForeground() const;
    const Wt::WBrush& getBackground() const;
    const Wt::WPen& getBackgroundBorder() const;

    bool testBackgroundFlag(EBackgroundStyle flag) const;

    std::string UTF8;
    EForegroundStyle foregroundStyle;
    EBackgroundStyle backgroundStyle;
};

typedef std::vector<Character> TTextLine;
typedef std::vector<TTextLine> TTextEditor;

class Rect {
public:
    Rect();

    void set_dimensions(double x1, double y1, double x2, double y2);

    double get_x1();
    double get_x2();
    double get_y1();
    double get_y2();

    double width();
    double height();

    void setBackgroundColor(const Wt::WColor& color);
    void setBackgroundStyle(const Wt::BrushStyle& style);
    void setBorderColor(const Wt::WColor& color);

    void draw(Wt::WPainter& painter);

    static bool isPointInRect(double x1, double y1, double x2, double y2, int x, int y);
    bool isPointInRect(int x, int y);

    virtual bool handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleClick(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseDown(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseUp(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e);

private:
    void initPen();

    void initBrush();

    double x1_, y1_, x2_, y2_;
    Wt::WPen pen;
    Wt::WBrush brush;

    Wt::WColor backgroundColor{ Wt::StandardColor::White };
    Wt::WColor borderColor{ Wt::StandardColor::Black };
    Wt::BrushStyle backgroundStyle{ Wt::BrushStyle::Solid };
};

class RowNumbers : public Rect {
public:
    RowNumbers();

    void draw(Wt::WPainter& painter,
        std::size_t row_no,
        std::size_t v_scroll,
        std::size_t current_row);

    virtual bool handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e);

    std::size_t padding_{ 2 };
    Wt::WFont font;
};

enum class ScrollDirection { HORIZONTAL, VERTICAL };

class CursorPos {
public:
    std::size_t row{ 0 };
    std::size_t col{ 0 };

    void set(std::size_t row, std::size_t col);

    bool operator>(const CursorPos& other) const;
    bool operator<(const CursorPos& other) const;
    bool operator==(const CursorPos& other) const;
    bool operator!=(const CursorPos& other) const;
};

class Scroll : public Rect {
public:
    Scroll(ScrollDirection direction, CodeEditor* scrollArea);

    virtual void draw(Wt::WPainter& painter);

    void scroll(int delta);

    void maximize_range(std::size_t new_range);

    bool handleMouseMove(const Wt::WMouseEvent& e);
    bool handleClick(const Wt::WMouseEvent& e);
    bool handleMouseDown(const Wt::WMouseEvent& e);
    bool handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e);
    bool handleMouseWheel(const Wt::WMouseEvent& e);

private:
    bool inLeftTrack(int x, int y);
    bool inRightTrack(int x, int y);

public:
    CodeEditor* scrollArea;
    ScrollDirection direction;
    std::size_t scroll_{ 0 };
    std::size_t range_{ 0 };
    std::size_t visible_range_{ 0 };
    double content_range_{ 0 }; // The space in pixels available for drawing (eg. width() - 2*padding)
    Wt::WFont font;
    Wt::WPen pen;
    double thumb_thickenss{ 5 };
    double thumb_pixel{ 0 }, left_track_pixel{ 0 }, right_track_pixel{ 0 }; // Updated in the 'draw' method.

    // Used to calculate draging the scroll thumb
    CursorPos thumb_last_left_down;
    std::size_t last_down_scroll{ 0 };

    double factor{ 0. }; // The amount of pixels  per character
};

class TextArea : public Rect {
public:
    TextArea();

    void draw(Wt::WPainter& painter,
        const TTextEditor& txt_editor,
        std::size_t current_row,
        std::size_t current_col,
        Scroll& v_scroll,
        Scroll& h_scroll,
        bool& reallocate_cursor,
        bool& repaint);

    std::size_t visibleRows();

    virtual bool handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleClick(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseDown(CodeEditor& parent, const Wt::WMouseEvent& e);
    virtual bool handleMouseWheel(CodeEditor& parent, const Wt::WMouseEvent& e);

    CursorPos mouseToRowCol(CodeEditor& parent, int mouse_x, int mouse_y);
    bool isMouseOverSelected(CodeEditor& parent, int x, int y);

    double padding_{ 2 };
    Wt::WFont font;
    Wt::WPen cursor_pen;
    double font_size{ 0. }; // Updated in 'draw' method called by the paint event of the parent class (canvas).
    double char_width{ 0. }; // Updated in 'draw' method
    CursorPos txt_last_left_down;
};

class StatusPanel : public Rect {
public:
    StatusPanel();

    void draw(Wt::WPainter& painter,
        const std::string txt);

private:
    double padding_{ 2 };
    Wt::WFont font;
};

class Selection {
public:
    bool is_selected{ false };
    CursorPos selection_start;
    CursorPos selection_end;

    void reset();
    
    void select(const CursorPos& pos_before, const CursorPos& pos_after);
};

class CodeEditor : public Wt::WPaintedWidget
{
public:
    CodeEditor();

    virtual void layoutSizeChanged(int width, int height);

    Wt::JSignal<std::string>& update_from_clipboard();
    void setTextFromClipboard(const Wt::WString& text);
    Scroll& getHorizScroll();
    Scroll& getVertScroll();
    CursorPos& getCurrentPos();
    void setCurrentPos(std::size_t row, std::size_t col);
    void setCurrentPos(CursorPos& pos);
    const TTextEditor& getTxt_editor() const;
    void resetSelection();
    void updateSelection(const CursorPos& old_pos, const CursorPos& new_pos);
    CursorPos mouseTextPosToTextPos(const CursorPos& pos);

    bool reallocate_cursor{ false };

    std::string text();

protected:
    void paintEvent(Wt::WPaintDevice* paintDevice);

private:
    void setLayout();

    std::string compose_status_string();

    void handleClick(const Wt::WMouseEvent& e);
    void handleMouseMove(const Wt::WMouseEvent& e);
    void handleMouseDrag(const Wt::WMouseEvent& e);
    void handleMouseWheel(const Wt::WMouseEvent& e);
    void handleKeyPressed(const Wt::WKeyEvent& e);
    void handleMouseDown(const Wt::WMouseEvent& e);
    void handleMouseUp(const Wt::WMouseEvent& e);

    void insertNewLine();
    void insertNewChar(const std::string& s);

    void handleKeyDown(const Wt::WKeyEvent& e);
    void handleKeyUp(const Wt::WKeyEvent& e);

    void maximize_range(Scroll& scroll);

    void putToClipboard();
    void getFromClipboard();

    static EBackgroundStyle setFlag(EBackgroundStyle base, EBackgroundStyle flag);
    static EBackgroundStyle removeFlag(EBackgroundStyle base, EBackgroundStyle flag);

    void setSelection(EBackgroundStyle style, EBackgroundStyle oper(EBackgroundStyle, EBackgroundStyle));
    void deleteSelected();

    std::string getTextInRange(std::size_t row_start, std::size_t col_start, std::size_t row_end, std::size_t col_end);
    std::string selectedText();

    TTextEditor txt_editor{ {} };
    CursorPos current_pos;

    Selection selection;

    RowNumbers row_numbers;
    Scroll horiz_scroll;
    Scroll vert_scroll;
    Rect scroll_dead;
    TextArea text_area;
    StatusPanel status_panel;

    Wt::JSignal<std::string> update_from_clipboard_;

    double h_scroll_height = 15;
    double v_scroll_width = 15;
    double row_numbers_panel_width = 40;
    double status_panel_height = 20;
    int width_{ 0 }, height_{ 0 };

    bool isDragMode{ false };
    Rect* dragComponent{ NULL };
};
