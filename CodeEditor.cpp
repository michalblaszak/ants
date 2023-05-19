#include "CodeEditor.h"

Wt::WBrush BackgroundNormal(Wt::WColor(255, 255, 255, 0));
Wt::WPen BackgroundBorderNormal(Wt::WColor(255, 255, 255, 0));
Wt::WPen ForegroundNormal(Wt::WColor(0, 0, 0));

Wt::WBrush BackgroundSelected(Wt::WColor(100, 180, 255));
Wt::WPen BackgroundBorderSelected(Wt::WColor(100, 180, 255));

Wt::WBrush BackgroundKeyword(Wt::WColor(255, 255, 255, 0));
Wt::WPen BackgroundBorderKeyword(Wt::WColor(255, 255, 255, 0));
Wt::WPen ForegroundKeyword(Wt::WColor(255, 0, 255));

Wt::WPen CurrentLineBorder(Wt::WColor(230, 230, 230));
Wt::WBrush CurrentLineBackground(Wt::WColor(230, 230, 230));

Wt::WPen RowNumbersCurrentLine(Wt::WColor(255, 255, 0));
Wt::WPen RowNumbersNormalLine(Wt::WColor(255, 255, 255));


namespace {
    std::ostream& operator<<(std::ostream& o, Wt::WMouseEvent::Button b)
    {
        switch (b) {
        case Wt::MouseButton::None:
            return o << "No button";
        case Wt::MouseButton::Left:
            return o << "LeftButton";
        case Wt::MouseButton::Right:
            return o << "RightButton";
        case Wt::MouseButton::Middle:
            return o << "MiddleButton";
        default:
            return o << "Unknown Button";
        }
    }

    std::ostream& operator<<(std::ostream& o, Wt::Key k)
    {
        switch (k) {
        default:
        case Wt::Key::Unknown: return o << "Key_unknown";
        case Wt::Key::Enter: return o << "Key_Enter";
        case Wt::Key::Tab: return o << "Key_Tab";
        case Wt::Key::Backspace: return o << "Key_Backspace";
        case Wt::Key::Shift: return o << "Key_Shift";
        case Wt::Key::Control: return o << "Key_Control";
        case Wt::Key::Alt: return o << "Key_Alt";
        case Wt::Key::PageUp: return o << "Key_PageUp";
        case Wt::Key::PageDown: return o << "Key_PageDown";
        case Wt::Key::End: return o << "Key_End";
        case Wt::Key::Home: return o << "Key_Home";
        case Wt::Key::Left: return o << "Key_Left";
        case Wt::Key::Up: return o << "Key_Up";
        case Wt::Key::Right: return o << "Key_Right";
        case Wt::Key::Down: return o << "Key_Down";
        case Wt::Key::Insert: return o << "Key_Insert";
        case Wt::Key::Delete: return o << "Key_Delete";
        case Wt::Key::Escape: return o << "Key_Escape";
        case Wt::Key::F1: return o << "Key_F1";
        case Wt::Key::F2: return o << "Key_F2";
        case Wt::Key::F3: return o << "Key_F3";
        case Wt::Key::F4: return o << "Key_F4";
        case Wt::Key::F5: return o << "Key_F5";
        case Wt::Key::F6: return o << "Key_F6";
        case Wt::Key::F7: return o << "Key_F7";
        case Wt::Key::F8: return o << "Key_F8";
        case Wt::Key::F9: return o << "Key_F9";
        case Wt::Key::F10: return o << "Key_F10";
        case Wt::Key::F11: return o << "Key_F11";
        case Wt::Key::F12: return o << "Key_F12";
        case Wt::Key::Space: return o << "Key_Space";
        case Wt::Key::A: return o << "Key_A";
        case Wt::Key::B: return o << "Key_B";
        case Wt::Key::C: return o << "Key_C";
        case Wt::Key::D: return o << "Key_D";
        case Wt::Key::E: return o << "Key_E";
        case Wt::Key::F: return o << "Key_F";
        case Wt::Key::G: return o << "Key_G";
        case Wt::Key::H: return o << "Key_H";
        case Wt::Key::I: return o << "Key_I";
        case Wt::Key::J: return o << "Key_J";
        case Wt::Key::K: return o << "Key_K";
        case Wt::Key::L: return o << "Key_L";
        case Wt::Key::M: return o << "Key_M";
        case Wt::Key::N: return o << "Key_N";
        case Wt::Key::O: return o << "Key_O";
        case Wt::Key::P: return o << "Key_P";
        case Wt::Key::Q: return o << "Key_Q";
        case Wt::Key::R: return o << "Key_R";
        case Wt::Key::S: return o << "Key_S";
        case Wt::Key::T: return o << "Key_T";
        case Wt::Key::U: return o << "Key_U";
        case Wt::Key::V: return o << "Key_V";
        case Wt::Key::W: return o << "Key_W";
        case Wt::Key::X: return o << "Key_X";
        case Wt::Key::Y: return o << "Key_Y";
        case Wt::Key::Z: return o << "Key_Z";
        }
    }

    std::ostream& operator<<(std::ostream& o, Wt::Coordinates c)
    {
        return o << c.x << ", " << c.y;
    }
    std::string modifiersToString(const Wt::WFlags< Wt::KeyboardModifier >& modifiers)
    {
        std::stringstream o;
        if (modifiers.test(Wt::KeyboardModifier::Shift)) o << "Shift ";
        if (modifiers.test(Wt::KeyboardModifier::Control)) o << "Control ";
        if (modifiers.test(Wt::KeyboardModifier::Alt)) o << "Alt ";
        if (modifiers.test(Wt::KeyboardModifier::Meta)) o << "Meta ";
        if (modifiers.empty()) o << "No modifiers";
        return o.str();
    }
}

/***************************************
 * Character
 **************************************/

Character::Character() : foregroundStyle(EForegroundStyle::NORMAL), backgroundStyle(EBackgroundStyle::NORMAL) {
    UTF8 = "";
}

Character::Character(std::string utf8, EForegroundStyle foreStyle, EBackgroundStyle backStyle) : UTF8(utf8), foregroundStyle(foreStyle), backgroundStyle(backStyle) {
}

Character::Character(const Character& other) {
    UTF8 = other.UTF8;
    foregroundStyle = other.foregroundStyle;
    backgroundStyle = other.backgroundStyle;
}

Character& Character::operator=(const Character& other) {
    UTF8 = other.UTF8;
    foregroundStyle = other.foregroundStyle;
    backgroundStyle = other.backgroundStyle;

    return *this;
}

bool Character::operator==(const Character& other) {
    return UTF8 == other.UTF8;
}

bool Character::isSameStyle(const Character& other) const {
    return foregroundStyle == other.foregroundStyle and backgroundStyle == other.backgroundStyle;
}

const Wt::WPen& Character::getForeground() const {
    switch (foregroundStyle) {
    case EForegroundStyle::NORMAL:  return ForegroundNormal;
    case EForegroundStyle::KEYWORD: return ForegroundKeyword;
    }
}

bool Character::testBackgroundFlag(EBackgroundStyle flag) const {
    return static_cast<int>(backgroundStyle) & static_cast<int>(flag);
}

const Wt::WBrush& Character::getBackground() const {
    if (testBackgroundFlag(EBackgroundStyle::SELECTED)) {
        return BackgroundSelected;
    }
    else {
        switch (backgroundStyle) {
        case EBackgroundStyle::NORMAL:   return BackgroundNormal;
        case EBackgroundStyle::KEYWORD:  return BackgroundKeyword;
        }
    }
}

const Wt::WPen& Character::getBackgroundBorder() const {
    if (testBackgroundFlag(EBackgroundStyle::SELECTED)) {
        return BackgroundBorderSelected;
    }
    else {
        switch (backgroundStyle) {
        case EBackgroundStyle::NORMAL:   return BackgroundBorderNormal;
        case EBackgroundStyle::KEYWORD:  return BackgroundBorderKeyword;
        }
    }
}


/***************************************
 * Rect
 **************************************/

Rect::Rect() {
    initPen();
    initBrush();
}

void Rect::set_dimensions(double x1, double y1, double x2, double y2) {
    x1_ = x1;
    x2_ = x2;
    y1_ = y1;
    y2_ = y2;
}

double Rect::get_x1() { return x1_; }
double Rect::get_x2() { return x2_; }
double Rect::get_y1() { return y1_; }
double Rect::get_y2() { return y2_; }

double Rect::width() { return x2_ - x1_; }
double Rect::height() { return y2_ - y1_; }

void Rect::setBackgroundColor(const Wt::WColor& color) {
    backgroundColor = color;
    brush.setColor(backgroundColor);
}

void Rect::setBackgroundStyle(const Wt::BrushStyle& style) {
    backgroundStyle = style;
    brush.setStyle(backgroundStyle);
}

void Rect::setBorderColor(const Wt::WColor& color) {
    borderColor = color;
    pen.setColor(borderColor);
}

void Rect::draw(Wt::WPainter& painter) {
    painter.save();

    painter.setPen(pen);

    painter.drawRect(x1_, y1_, width(), height());
    painter.fillRect(x1_, y1_, width(), height(), brush);

    painter.restore();
}

bool Rect::isPointInRect(double x1, double y1, double x2, double y2, int x, int y) {
    return x >= x1 and x <= x2 and y >= y1 and y <= y2;
}
bool Rect::isPointInRect(int x, int y) {
    return isPointInRect(get_x1(), get_y1(), get_x2(), get_y2(), x, y);
}

bool Rect::handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (isPointInRect(e.widget().x, e.widget().y)) {
        parent.decorationStyle().setCursor(Wt::Cursor::Arrow);

        return true;
    }
    return false;
}

bool Rect::handleClick(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (isPointInRect(e.widget().x, e.widget().y)) {
        return true;
    }
    return false;
}

bool Rect::handleMouseDown(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (isPointInRect(e.widget().x, e.widget().y)) {
        return true;
    }
    return false;
}

bool Rect::handleMouseUp(CodeEditor& parent, const Wt::WMouseEvent& e) {
    return true;
}

bool Rect::handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e) {
    return true;
}

void Rect::initPen() {
    pen.setColor(borderColor);
}

void Rect::initBrush() {
    brush.setColor(backgroundColor);
    brush.setStyle(backgroundStyle);
}

/***************************************
 * RowNumbers
 **************************************/

RowNumbers::RowNumbers() {
    font.setFamily(Wt::FontFamily::Monospace, "'Lucida Console'");
    font.setSize(12);
}

void RowNumbers::draw(Wt::WPainter& painter,
    std::size_t row_no,
    std::size_t v_scroll,
    std::size_t current_row)
{
    Rect::draw(painter);

    painter.save();
    painter.setFont(font);

    double font_size = painter.font().sizeLength().value();

    for (std::size_t i = v_scroll; i < row_no; i++) {
        std::string s = std::to_string(i + 1);

        if (i == current_row) {
            painter.setPen(RowNumbersCurrentLine);
        }
        else {
            painter.setPen(RowNumbersNormalLine);
        }

        painter.drawText(get_x1() + padding_, get_y1() + (i - v_scroll) * font_size + padding_,
            width() - 2. * padding_, font_size, Wt::AlignmentFlag::Right | Wt::AlignmentFlag::Top, s);

    }

    painter.restore();
}

bool RowNumbers::handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (e.widget().x >= get_x1() and e.widget().x <= get_x2() and e.widget().y >= get_y1() and e.widget().y <= get_y2()) {
        parent.decorationStyle().setCursor(Wt::Cursor::Arrow);

        return true;
    }
    return false;
}

/***************************************
 * Scroll
 **************************************/

Scroll::Scroll(ScrollDirection direction, CodeEditor* scrollArea) : direction(direction), scrollArea(scrollArea) {
    font.setFamily(Wt::FontFamily::Monospace, "'Lucida Console'");
    font.setSize(12);

    pen.setCapStyle(Wt::PenCapStyle::Round);
    pen.setColor(Wt::StandardColor::Gray);
    pen.setStyle(Wt::PenStyle::SolidLine);
    pen.setWidth(thumb_thickenss);
}

void Scroll::draw(Wt::WPainter& painter) {
    Rect::draw(painter);
    painter.save();

    painter.setFont(font); // Needed to calculate visible_range correctly

    double scroll_length = direction == ScrollDirection::HORIZONTAL ? width() : height();
    double slide_area_width = scroll_length - thumb_thickenss;
    visible_range_ = content_range_ / (direction == ScrollDirection::HORIZONTAL ? (painter.device()->measureText("w").width() / 1.02) : font.sizeLength().value());
    int right_characters = range_ - scroll_ - visible_range_;
    std::size_t empty_chars = right_characters < 0 ? -right_characters : 0;
    factor = range_ == 0 ? 0 : (slide_area_width / (range_ + empty_chars));

    left_track_pixel = scroll_ * factor;
    right_track_pixel = (right_characters < 0 ? 0 : right_characters) * factor;
    thumb_pixel = slide_area_width - left_track_pixel - right_track_pixel;

    painter.setPen(pen);

    if (direction == ScrollDirection::HORIZONTAL) {
        painter.drawLine(get_x1() + thumb_thickenss / 2 + left_track_pixel, get_y1() + height() / 2,
            get_x1() + thumb_thickenss / 2 + left_track_pixel + thumb_pixel, get_y1() + height() / 2);
    }
    else { // VERTICAL
        painter.drawLine(get_x1() + width() / 2, get_y1() + thumb_thickenss / 2 + left_track_pixel,
            get_x1() + width() / 2, get_y1() + thumb_thickenss / 2 + left_track_pixel + thumb_pixel);
    }

    painter.restore();
}

void Scroll::scroll(int delta) {
    
    if ( (delta < 0 and scroll_ >= -delta) 
        or (delta >= 0 and scroll_ + delta <= range_) )
    {
        scroll_ += delta;
    }
}

void Scroll::maximize_range(std::size_t new_range) {
    if (range_ < new_range) range_ = new_range;
}

bool Scroll::handleMouseMove(const Wt::WMouseEvent& e) {
    if (e.widget().x >= get_x1() and e.widget().x <= get_x2() and e.widget().y >= get_y1() and e.widget().y <= get_y2()) {
        scrollArea->decorationStyle().setCursor(Wt::Cursor::Arrow);

        return true;
    }
    return false;
}

bool Scroll::handleClick(const Wt::WMouseEvent& e) {
    if (inLeftTrack(e.widget().x, e.widget().y)) {
        scroll_ = scroll_ <= visible_range_ ? 0 : scroll_ - visible_range_;

        scrollArea->update();

        return true;
    }
    else if (inRightTrack(e.widget().x, e.widget().y)) {
        scroll_ = (scroll_ + 2 * visible_range_) >= range_
            ? range_ - visible_range_
            : scroll_ + visible_range_;

        scrollArea->update();

        return true;
    }

    return false;
}

bool Scroll::inLeftTrack(int x, int y) {
    if (direction == ScrollDirection::HORIZONTAL) {
        return x >= get_x1() and x < get_x1() + left_track_pixel and y >= get_y1() and y <= get_y2();
    }
    else { // VERTICAL
        return x >= get_x1() and x <= get_x2() and y >= get_y1() and y < get_y1() + left_track_pixel;
    }

    return false;
}

bool Scroll::inRightTrack(int x, int y) {
    if (direction == ScrollDirection::HORIZONTAL) {
        return x >= get_x1() + left_track_pixel + thumb_pixel and x < get_x2() and y >= get_y1() and y <= get_y2();
    }
    else { // VERTICAL
        return x >= get_x1() and x <= get_x2() and y >= get_y1() + left_track_pixel + thumb_pixel and y < get_y2();
    }

    return false;
}

bool Scroll::handleMouseDown(const Wt::WMouseEvent& e) {
    if (Rect::handleMouseDown(*scrollArea, e)) {
        if (direction == ScrollDirection::HORIZONTAL) {
            // Are we in a thumb?
            if (isPointInRect(get_x1() + left_track_pixel, get_y1(), get_x1() + left_track_pixel + thumb_pixel, get_y2(), e.widget().x, e.widget().y)) {
                if (e.button() == Wt::MouseButton::Left) {
                    thumb_last_left_down.set(e.widget().y, e.widget().x);
                    last_down_scroll = scroll_;

                    return true;
                }
            }
        }
        else { // VERTICAL
            // Are we in a thumb?
            if (isPointInRect(get_x1(), get_y1() + left_track_pixel, get_x2(), get_y2() + left_track_pixel + thumb_pixel, e.widget().x, e.widget().y)) {
                if (e.button() == Wt::MouseButton::Left) {
                    thumb_last_left_down.set(e.widget().y, e.widget().x);
                    last_down_scroll = scroll_;

                    return true;
                }
            }
        }
    }

    return false;
}

bool Scroll::handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (e.button() == Wt::MouseButton::Left) {
        if (direction == ScrollDirection::HORIZONTAL) {
            int x_delta = e.widget().x - thumb_last_left_down.col;
            double chars = x_delta / factor;
            //thumb_last_left_down.col = e.widget().x;

            scroll_ = (chars < 0 and last_down_scroll < -chars) ? 0 : last_down_scroll + chars;

            std::cout << "factor=" << factor << ", mouse_x=" << e.widget().x << ", thumb_last_left_down.col=" << thumb_last_left_down.col << std::endl;
            std::cout << "x_delta=" << x_delta << ", chars=" << chars << ", scroll_=" << scroll_ << std::endl;
        }
        else { // VERTICAL
            int y_delta = e.widget().y - thumb_last_left_down.row;
            double chars = y_delta / factor;
            //thumb_last_left_down.col = e.widget().x;

            scroll_ = (chars < 0 and last_down_scroll < -chars) ? 0 : last_down_scroll + chars;

            std::cout << "factor=" << factor << ", mouse_x=" << e.widget().x << ", thumb_last_left_down.col=" << thumb_last_left_down.col << std::endl;
            std::cout << "x_delta=" << y_delta << ", chars=" << chars << ", scroll_=" << scroll_ << std::endl;
        }

        scrollArea->update();
        return true;
    }

    return false;
}

bool Scroll::handleMouseWheel(const Wt::WMouseEvent& e) {
    if (isPointInRect(e.widget().x, e.widget().y)) {
        scroll(-e.wheelDelta());

        scrollArea->update();

        return true;
    }

    return false;
}

/***************************************
 * TextArea
 **************************************/

TextArea::TextArea() {
    font.setFamily(Wt::FontFamily::Monospace, "'Lucida Console'");
    font.setSize(12);

    cursor_pen.setColor(Wt::StandardColor::Black);
}

void TextArea::draw(Wt::WPainter& painter,
    const TTextEditor& txt_editor,
    std::size_t current_row,
    std::size_t current_col,
    Scroll& v_scroll,
    Scroll& h_scroll,
    bool& reallocate_cursor,
    bool& repaint)
{
    Rect::draw(painter);

    painter.save();

    painter.setFont(font);

    char_width = painter.device()->measureText("w").width() / 1.02;

    Wt::WPainterPath clipPath;
    clipPath.addRect(get_x1() + padding_, get_y1() + padding_, width() - 2 * padding_, height() - 2 * padding_);
    painter.setClipPath(clipPath);
    painter.setClipping(true);

    font_size = painter.font().sizeLength().value();
    std::size_t row_ = 0;

    for (row_ = 0 + v_scroll.scroll_; row_ < txt_editor.size(); row_++) {
        std::size_t row_vscroll = row_ - v_scroll.scroll_;
        if (get_y1() + (row_vscroll + 1.) * font_size + padding_ > get_y2() - padding_) {
            break;
        }

        std::string s{ "" };
        std::string complete_s{ "" };
        Character prev_char;
        int prev_char_idx = -1;
        double text_offset = 0.;

        // Draw the current line selector
        if (row_ == current_row) {
            painter.setPen(CurrentLineBorder);
            painter.setBrush(CurrentLineBackground);
            painter.drawRect(get_x1(), get_y1() + row_vscroll * font_size + padding_, width(), font_size);
        }

        for (std::size_t i = h_scroll.scroll_; i < txt_editor[row_].size(); i++) {
            const Character& current_char = txt_editor[row_][i];

            if (prev_char_idx == -1) {
                prev_char = current_char;
                s += current_char.UTF8;
                prev_char_idx = i;
            }
            else {
                if (prev_char.isSameStyle(current_char)) {
                    s += current_char.UTF8;
                }
                else {
                    complete_s += s;
                    double text_len = painter.device()->measureText(s).width() / 1.02;

                    // Draw background
                    painter.setBrush(prev_char.getBackground());
                    painter.setPen(prev_char.getBackgroundBorder());
                    painter.drawRect(get_x1() + padding_ + text_offset, get_y1() + row_vscroll * font_size + padding_,
                        text_len, font_size);

                    // Draw text
                    painter.setPen(prev_char.getForeground());
                    painter.drawText(get_x1() + padding_ + text_offset, get_y1() + row_vscroll * font_size + padding_,
                        width() - 2 * padding_ - text_offset, font_size,
                        Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Top, s);

                    text_offset += text_len;
                    s = current_char.UTF8;
                    prev_char_idx = i;
                    prev_char = current_char;
                }
            }

        }

        // Draw background
        double text_len = painter.device()->measureText(s).width() / 1.02;
        painter.setBrush(prev_char.getBackground());
        painter.setPen(prev_char.getBackgroundBorder());

        painter.drawRect(get_x1() + padding_ + text_offset, get_y1() + row_vscroll * font_size + padding_,
            text_len, font_size);

        // Draw text
        painter.setPen(prev_char.getForeground());
        complete_s += s;
        painter.drawText(get_x1() + padding_ + text_offset, get_y1() + row_vscroll * font_size + padding_,
            width() - 2 * padding_ - text_offset, font_size,
            Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Top, s);

        // Draw Cursor
        if (row_ == current_row) {
            std::string left_s{ "" };
            int str_len = txt_editor[row_].size();

            if (str_len == current_col)
                left_s = complete_s;
            else {
                for (std::size_t c_pos_ = h_scroll.scroll_; c_pos_ < current_col; c_pos_++) {
                    left_s += txt_editor[row_][c_pos_].UTF8;
                }
            }

            if (current_col >= h_scroll.scroll_) { // If cusror is not beyond the left border
                double x = painter.device()->measureText(left_s).width() / 1.02;
                painter.setPen(cursor_pen);
                painter.drawLine(get_x1() + x + padding_, get_y1() + row_vscroll * font_size + padding_,
                    get_x1() + x + padding_, get_y1() + (row_vscroll + 1.) * font_size + padding_);
            }
        }
    }

    painter.restore();

    repaint = false;

    if (reallocate_cursor) {
        reallocate_cursor = false;

        if (current_row < v_scroll.scroll_) {
            v_scroll.scroll_ = current_row;
            repaint = true;
        }
        else if (current_row >= row_) {
            v_scroll.scroll_ += current_row - (row_ - 1);
            repaint = true;
        }

        if (current_col < h_scroll.scroll_) {
            h_scroll.scroll_ = current_col;
            repaint = true;
        }
        else if (current_col >= h_scroll.visible_range_ + h_scroll.scroll_) {
            h_scroll.scroll_ = current_col - h_scroll.visible_range_;
            repaint = true;
        }
    }
}

std::size_t TextArea::visibleRows() {
    return height() / font_size;
}

bool TextArea::handleMouseMove(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (e.widget().x >= get_x1() and e.widget().x <= get_x2() and e.widget().y >= get_y1() and e.widget().y <= get_y2()) {
        if (isMouseOverSelected(parent, e.widget().x, e.widget().y)) {
            parent.decorationStyle().setCursor(Wt::Cursor::Arrow);
        }
        else {
            parent.decorationStyle().setCursor(Wt::Cursor::IBeam);
        }

        return true;
    }
    return false;
}

CursorPos TextArea::mouseToRowCol(CodeEditor& parent, int mouse_x, int mouse_y) {
    CursorPos pos;

    pos.col = std::max(0l, lround(((double)mouse_x - get_x1() - padding_) / char_width) + (long)(parent.getHorizScroll().scroll_));
    pos.row = std::max(0l, lround(((double)mouse_y - get_y1() - padding_ - font_size / 2) / font_size) + (long)(parent.getVertScroll().scroll_));

    return pos;
}

bool TextArea::handleClick(CodeEditor& parent, const Wt::WMouseEvent& e)
{
    if (Rect::handleClick(parent, e)) {
        parent.resetSelection();

        CursorPos pos = mouseToRowCol(parent, e.widget().x, e.widget().y);
        CursorPos txt_pos = parent.mouseTextPosToTextPos(pos);

        std::cout << "r=" << pos.row << ", c=" << pos.col << std::endl;

        parent.setCurrentPos(txt_pos.row, txt_pos.col);
        parent.update();

        return true;
    }

    return false;
}

bool TextArea::isMouseOverSelected(CodeEditor& parent, int mouse_x, int mouse_y) {
    CursorPos pos = mouseToRowCol(parent, mouse_x, mouse_y);

    if ((pos.row < parent.getTxt_editor().size())
        and pos.col < parent.getTxt_editor()[pos.row].size()
        ) {
        return parent.getTxt_editor()[pos.row][pos.col].testBackgroundFlag(EBackgroundStyle::SELECTED);
    }
    else {
        return false;
    }
}

bool TextArea::handleMouseDown(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (Rect::handleMouseDown(parent, e)) {
        if (e.button() == Wt::MouseButton::Left) {
            CursorPos pos = mouseToRowCol(parent, e.widget().x, e.widget().y);
            txt_last_left_down = parent.mouseTextPosToTextPos(pos);
        }
        else { // Other buttons
            // TODO
        }

        return true;
    }

    return false;
}

bool TextArea::handleMouseDrag(CodeEditor& parent, const Wt::WMouseEvent& e) {
    parent.resetSelection();

    CursorPos pos = mouseToRowCol(parent, e.widget().x, e.widget().y);

    CursorPos txt_end = parent.mouseTextPosToTextPos(pos);

    parent.updateSelection(txt_last_left_down, txt_end);
    parent.reallocate_cursor = true;
    parent.setCurrentPos(txt_end);
    parent.update();

    return true;
}

bool TextArea::handleMouseWheel(CodeEditor& parent, const Wt::WMouseEvent& e) {
    if (isPointInRect(e.widget().x, e.widget().y)) {
        parent.getVertScroll().scroll(-e.wheelDelta());

        parent.update();

        return true;
    }

    return false;
}

/***************************************
 * StatusPanel
 **************************************/

StatusPanel::StatusPanel() {
    font.setFamily(Wt::FontFamily::SansSerif, "Helvetica");
    font.setSize(10);
}

void StatusPanel::draw(Wt::WPainter& painter,
    const std::string txt
)
{
    Rect::draw(painter);

    painter.save();

    painter.setFont(font);

    Wt::WPainterPath clipPath;
    clipPath.addRect(get_x1() + padding_, get_y1() + padding_, width() - 2 * padding_, height() - 2 * padding_);
    painter.setClipPath(clipPath);
    painter.setClipping(true);

    double font_size = painter.font().sizeLength().value();

    painter.drawText(get_x1() + padding_, get_y1() + padding_,
        width() - 2 * padding_, height() - 2 * padding_,
        Wt::AlignmentFlag::Left | Wt::AlignmentFlag::Middle, txt);

    painter.restore();
}

/***************************************
 * CursorPos
 **************************************/

void CursorPos::set(std::size_t row, std::size_t col) {
    this->row = row;
    this->col = col;
}

bool CursorPos::operator>(const CursorPos& other) const {
    return this->row > other.row or
        this->row == other.row and this->col > other.col;
}

bool CursorPos::operator<(const CursorPos& other) const {
    return this->row < other.row or
        this->row == other.row and this->col < other.col;
}

bool CursorPos::operator==(const CursorPos& other) const {
    return this->row == other.row and this->col == other.col;
}

bool CursorPos::operator!=(const CursorPos& other) const {
    return this->row != other.row or this->col != other.col;
}

/***************************************
 * Selection
 **************************************/

void Selection::reset() {
    is_selected = false;
    selection_start.set(0, 0);
    selection_end.set(0, 0);
}

void Selection::select(const CursorPos& pos_before, const CursorPos& pos_after) {
    bool cursor_on_the_right = pos_before == selection_end;
    bool cursor_on_the_left = pos_before == selection_start;
    bool move_left = pos_after < pos_before;
    bool move_right = pos_after > pos_before;

    if (!is_selected and move_right) {
        is_selected = true;
        selection_start = pos_before;
        selection_end = pos_after;
    }
    else if (!is_selected and move_left) {
        is_selected = true;
        selection_start = pos_after;
        selection_end = pos_before;
    }
    else if (is_selected and cursor_on_the_right and move_right) {
        selection_end = pos_after;
    }
    else if (is_selected and cursor_on_the_right and move_left and pos_after > selection_start) {
        selection_end = pos_after;
    }
    else if (is_selected and cursor_on_the_right and move_left and pos_after < selection_start) {
        selection_end = selection_start;
        selection_start = pos_after;
    }
    else if (is_selected and cursor_on_the_right and pos_after == selection_start) {
        reset();
    }
    else if (is_selected and cursor_on_the_left and pos_after == selection_end) {
        reset();
    }
    else if (is_selected and cursor_on_the_left and move_right and pos_after > selection_end) {
        selection_start = selection_end;
        selection_end = pos_after;
    }
    else if (is_selected and cursor_on_the_left and move_right and pos_after < selection_end) {
        selection_start = pos_after;
    }
    else if (is_selected and cursor_on_the_left and move_left) {
        selection_start = pos_after;
    }
}

/***************************************
 * CodeEditor
 **************************************/

CodeEditor::CodeEditor()
    : Wt::WPaintedWidget(),
    horiz_scroll(ScrollDirection::HORIZONTAL, this),
    vert_scroll(ScrollDirection::VERTICAL, this),
    update_from_clipboard_(this, "update_from_clipboard")
{
    this->setLayoutSizeAware(true);

    //resize(400, 200); // provide a default size
    this->setTabIndex(1);

    this->keyPressed().connect(this, &CodeEditor::handleKeyPressed);
    this->keyWentDown().connect(this, &CodeEditor::handleKeyDown);
    this->keyWentUp().connect(this, &CodeEditor::handleKeyUp);
    this->clicked().connect(this, &CodeEditor::handleClick);
    this->mouseMoved().connect(this, &CodeEditor::handleMouseMove);
    this->mouseDragged().connect(this, &CodeEditor::handleMouseDrag);
    this->mouseWheel().connect(this, &CodeEditor::handleMouseWheel);
    this->mouseWentDown().connect(this, &CodeEditor::handleMouseDown);
    this->mouseWentUp().connect(this, &CodeEditor::handleMouseUp);

    this->focussed().connect(this, []() {std::cout << "Got focus" << std::endl; });

    //this->keyEventSignal();

    setLayout();

    row_numbers.setBackgroundColor(Wt::WColor(155, 155, 155));

    horiz_scroll.setBackgroundColor(Wt::WColor(230, 230, 230));
    horiz_scroll.setBorderColor(Wt::WColor(255, 255, 255, 255));

    vert_scroll.setBackgroundColor(Wt::WColor(230, 230, 230));
    vert_scroll.setBorderColor(Wt::WColor(255, 255, 255, 255));

    scroll_dead.setBackgroundColor(Wt::WColor(210, 210, 210));
    scroll_dead.setBorderColor(Wt::WColor(255, 255, 255, 255));

    text_area.setBackgroundColor(Wt::WColor(Wt::StandardColor::White));
    text_area.setBorderColor(Wt::WColor(255, 255, 255, 255));

    status_panel.setBackgroundColor(Wt::WColor(Wt::StandardColor::Gray));
    status_panel.setBorderColor(Wt::WColor(Wt::StandardColor::Black));

    update_from_clipboard_.connect(this, &CodeEditor::setTextFromClipboard);
}

const TTextEditor& CodeEditor::getTxt_editor() const {
    return txt_editor;
}

void CodeEditor::layoutSizeChanged(int width, int height) {

    Wt::WPaintedWidget::layoutSizeChanged(width, height);

    width_ = width;
    height_ = height;

    setLayout();
    std::cout << "layoutSizeChanged((" << width << "," << height << ")" << ":" << this->width().value() << "," << this->height().value() << std::endl;
}

Scroll& CodeEditor::getHorizScroll() {
    return horiz_scroll;
}

Scroll& CodeEditor::getVertScroll() {
    return vert_scroll;
}

CursorPos& CodeEditor::getCurrentPos() {
    return current_pos;
}

CursorPos CodeEditor::mouseTextPosToTextPos(const CursorPos& pos) {
    CursorPos ret;

    ret.row = std::min(pos.row, txt_editor.size() - 1);
    ret.col = std::min(pos.col, txt_editor[ret.row].size());

    return ret;
}

void CodeEditor::setCurrentPos(CursorPos& pos) {
    setCurrentPos(pos.row, pos.col);
}

void CodeEditor::setCurrentPos(std::size_t row, std::size_t col) {
    current_pos.row = row;
    current_pos.col = col;
}

Wt::JSignal<std::string>& CodeEditor::update_from_clipboard() {
    return update_from_clipboard_;
}

void CodeEditor::setTextFromClipboard(const Wt::WString& text) {
    std::string str_utf8 = text.toUTF8();
    const char* utf8_buf = str_utf8.c_str();
    std::size_t len = 0;

    while (*utf8_buf) {
        if (((*utf8_buf) & 0b10000000) != 0b10000000) { // One byte
            len = 1;
        }
        else if (((*utf8_buf) & 0b11000000) == 0b11000000) { // 2 bytes
            len = 2;
        }
        else if (((*utf8_buf) & 0b11100000) == 0b11100000) { // 3 bytes
            len = 3;
        }
        else if (((*utf8_buf) & 0b11110000) == 0b11110000) { // 4 bytes
            len = 4;
        }
        else {
            len = 0;
            std::cout << "Error: UFT-8 code point longer than 4 bytes." << std::endl;
        }

        if (len > 0) {
            if (*utf8_buf == '\r') {
                // Ignore it
            }
            else if (*utf8_buf == '\n') {
                insertNewLine();
            }
            else if (*utf8_buf == '\t') {
                insertNewChar(" ");
                insertNewChar(" ");
            }
            else {
                insertNewChar(std::string(utf8_buf, len));
            }

            utf8_buf += len;
        }
    } // ~while

//    horiz_scroll.maximize_range(txt_editor[current_pos.row].size());

    maximize_range(horiz_scroll);
    maximize_range(vert_scroll);

    reallocate_cursor = true;

    update();
}

void CodeEditor::paintEvent(Wt::WPaintDevice* paintDevice) {
    Wt::WPainter painter(paintDevice);
    bool repaint;

    do {
        row_numbers.draw(painter, txt_editor.size(), vert_scroll.scroll_, current_pos.row);
        text_area.draw(painter, txt_editor, current_pos.row, current_pos.col, vert_scroll, horiz_scroll, reallocate_cursor, repaint);
        horiz_scroll.draw(painter);
        vert_scroll.draw(painter);
        scroll_dead.draw(painter);
        status_panel.draw(painter, compose_status_string());
    } while (repaint);
}

void CodeEditor::setLayout() {
    row_numbers.set_dimensions(0, 0,
        row_numbers_panel_width, height_ - h_scroll_height - status_panel_height);

    horiz_scroll.set_dimensions(0, height_ - h_scroll_height - status_panel_height,
        width_ - v_scroll_width, height_ - status_panel_height);

    vert_scroll.set_dimensions(width_ - v_scroll_width, 0,
        width_, height_ - h_scroll_height - status_panel_height);

    scroll_dead.set_dimensions(width_ - v_scroll_width, height_ - h_scroll_height - status_panel_height,
        width_, height_ - status_panel_height);

    text_area.set_dimensions(row_numbers_panel_width, 0,
        width_ - v_scroll_width, height_ - h_scroll_height - status_panel_height);

    status_panel.set_dimensions(0, height_ - status_panel_height,
        width_, height_);

    // This must be initialized after text_area
    horiz_scroll.content_range_ = text_area.width() - 2 * text_area.padding_;
    vert_scroll.content_range_ = text_area.height() - 2 * text_area.padding_;
}

std::string CodeEditor::compose_status_string() {
    std::string ret;

    ret = "col: " + std::to_string(current_pos.col + 1) + "  row: " + std::to_string(current_pos.row + 1);

    return ret;
}

void CodeEditor::handleClick(const Wt::WMouseEvent& e) {
    std::cout << "Clicked: x=" << e.widget().x << ", y=" << e.widget().y << std::endl;
    row_numbers.handleClick(*this, e)
        or text_area.handleClick(*this, e)
        or horiz_scroll.handleClick(e)
        or vert_scroll.handleClick(e)
        or scroll_dead.handleClick(*this, e)
        or status_panel.handleClick(*this, e);
}

void CodeEditor::handleMouseMove(const Wt::WMouseEvent& e) {
    row_numbers.handleMouseMove(*this, e)
        or text_area.handleMouseMove(*this, e)
        or horiz_scroll.handleMouseMove(e)
        or vert_scroll.handleMouseMove(e)
        or scroll_dead.handleMouseMove(*this, e)
        or status_panel.handleMouseMove(*this, e);
}

void CodeEditor::handleMouseDown(const Wt::WMouseEvent& e) {
    if (text_area.handleMouseDown(*this, e)) {
        isDragMode = true;
        dragComponent = &text_area;
    }
    else if (horiz_scroll.handleMouseDown(e)) {
        isDragMode = true;
        dragComponent = &horiz_scroll;
    }
    else if (vert_scroll.handleMouseDown(e)) {
        isDragMode = true;
        dragComponent = &vert_scroll;
    }
}

void CodeEditor::handleMouseUp(const Wt::WMouseEvent& e) {
    isDragMode = false;
    dragComponent = NULL;
}

void CodeEditor::handleMouseDrag(const Wt::WMouseEvent& e) {
    std::cout << "Mouse drag: x=" << e.widget().x << ", y=" << e.widget().y << ", delta x=" << e.dragDelta().x << ", y=" << e.dragDelta().y << std::endl;

    if (isDragMode and dragComponent) {
        dragComponent->handleMouseDrag(*this, e);
    }
}

void CodeEditor::handleMouseWheel(const Wt::WMouseEvent& e) {
    std::cout << "Mouse wheel: x=" << e.widget().x << ", y=" << e.widget().y << ", wheelDelta=" << e.wheelDelta() << std::endl;

    text_area.handleMouseWheel(*this, e)
        or horiz_scroll.handleMouseWheel(e)
        or vert_scroll.handleMouseWheel(e);
}

void CodeEditor::handleKeyPressed(const Wt::WKeyEvent& e) {
    std::cout << "Pressed:" << e.key() << ":" << modifiersToString(e.modifiers()) << ":" << e.charCode() << ":" << e.text() << std::endl;

    if (selection.is_selected) {
        deleteSelected();
    }

    if (e.charCode() == 13) { // Enter
        insertNewLine();
    }
    else {
        insertNewChar(e.text().toUTF8());
    }

    maximize_range(vert_scroll);
    maximize_range(horiz_scroll);

    reallocate_cursor = true;

    update();
}

void CodeEditor::insertNewLine() {
    TTextLine new_line(txt_editor[current_pos.row].begin() + current_pos.col, txt_editor[current_pos.row].end());
    txt_editor.insert(txt_editor.begin() + current_pos.row + 1, new_line);
    txt_editor[current_pos.row].erase(txt_editor[current_pos.row].begin() + current_pos.col, txt_editor[current_pos.row].end());
    current_pos.row++;
    current_pos.col = 0;
}

void CodeEditor::insertNewChar(const std::string& s) {
    txt_editor[current_pos.row].insert(txt_editor[current_pos.row].begin() + current_pos.col, Character(s, EForegroundStyle::NORMAL, EBackgroundStyle::NORMAL));
    current_pos.col++;
}

void CodeEditor::handleKeyDown(const Wt::WKeyEvent& e) {
    std::cout << "Down:" << e.key() << ":" << modifiersToString(e.modifiers()) << ":" << e.charCode() << ":" << e.text() << std::endl;

    switch (e.key()) {
    case Wt::Key::Left: {
        CursorPos old_pos = current_pos;

        current_pos.col = current_pos.col == 0 ? 0 : current_pos.col - 1;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Right: {
        CursorPos old_pos = current_pos;

        current_pos.col = current_pos.col == txt_editor[current_pos.row].size() ? current_pos.col : current_pos.col + 1;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Home: {
        CursorPos old_pos = current_pos;

        if (e.modifiers().test(Wt::KeyboardModifier::Control)) {
            setCurrentPos(0, 0);
        }
        else {
            current_pos.col = 0;
        }

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::End: {
        CursorPos old_pos = current_pos;

        if (e.modifiers().test(Wt::KeyboardModifier::Control)) {
            current_pos.row = txt_editor.size() - 1;
            current_pos.col = txt_editor[current_pos.row].size();
        }
        else {
            current_pos.col = txt_editor[current_pos.row].size();
        }

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Up: {
        CursorPos old_pos = current_pos;

        current_pos.row = current_pos.row == 0 ? 0 : current_pos.row - 1;
        current_pos.col = current_pos.col > txt_editor[current_pos.row].size() ? txt_editor[current_pos.row].size() : current_pos.col;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Down: {
        CursorPos old_pos = current_pos;

        current_pos.row = current_pos.row == txt_editor.size() - 1 ? current_pos.row : current_pos.row + 1;
        current_pos.col = current_pos.col > txt_editor[current_pos.row].size() ? txt_editor[current_pos.row].size() : current_pos.col;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Delete: {
        if (selection.is_selected) {
            deleteSelected();
        }
        else {
            if (current_pos.col < txt_editor[current_pos.row].size()) {
                // Delete the character inside the line
                txt_editor[current_pos.row].erase(txt_editor[current_pos.row].begin() + current_pos.col);
//                maximize_range(horiz_scroll);
            }
            else if (current_pos.col == txt_editor[current_pos.row].size() and current_pos.row < txt_editor.size() - 1) {
                // If the cursor is at the end of the line and there is one more following line, then merge the lines
                txt_editor[current_pos.row].insert(txt_editor[current_pos.row].end(), txt_editor[current_pos.row + 1].begin(), txt_editor[current_pos.row + 1].end());
                txt_editor.erase(txt_editor.begin() + current_pos.row + 1);
//                maximize_range(horiz_scroll);
//                vert_scroll.range_--;
            }
        }

        maximize_range(horiz_scroll);
        maximize_range(vert_scroll);

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::Backspace: {
        if (selection.is_selected) {
            deleteSelected();
        }
        else {
            if (current_pos.col == 0 and current_pos.row > 0) {
                // Merge the line with its preceding line
                std::size_t new_current_col = txt_editor[current_pos.row - 1].size();
                txt_editor[current_pos.row - 1].insert(txt_editor[current_pos.row - 1].end(), txt_editor[current_pos.row].begin(), txt_editor[current_pos.row].end());
                txt_editor.erase(txt_editor.begin() + current_pos.row);
                current_pos.col = new_current_col;
                current_pos.row--;
//                maximize_range(horiz_scroll);
//                vert_scroll.range_--;
            }
            else if (current_pos.col > 0) {
                // Delete the character inside the line
                txt_editor[current_pos.row].erase(txt_editor[current_pos.row].begin() + current_pos.col - 1);
                current_pos.col--;
//                maximize_range(horiz_scroll);
            }
        }

        maximize_range(horiz_scroll);
        maximize_range(vert_scroll);

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::C: {
        if (e.modifiers().test(Wt::KeyboardModifier::Control)) {
            putToClipboard();
        }

        break;
    }
    case Wt::Key::V: {
        if (e.modifiers().test(Wt::KeyboardModifier::Control)) {
            if (selection.is_selected) {
                deleteSelected();
            }

            getFromClipboard(); // This is an asynchronous operation. Refressing scrollbars will happen in the clipboard event handler
        }

        break;
    }
    case Wt::Key::PageDown: {
        CursorPos old_pos = current_pos;

        current_pos.row = current_pos.row + text_area.visibleRows() > txt_editor.size() - 1
            ? txt_editor.size() - 1 : current_pos.row + text_area.visibleRows();
        current_pos.col = current_pos.col > txt_editor[current_pos.row].size() ? txt_editor[current_pos.row].size() : current_pos.col;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }
    case Wt::Key::PageUp: {
        CursorPos old_pos = current_pos;

        current_pos.row = current_pos.row < text_area.visibleRows()
            ? 0 : current_pos.row - text_area.visibleRows();
        current_pos.col = current_pos.col > txt_editor[current_pos.row].size() ? txt_editor[current_pos.row].size() : current_pos.col;

        if (e.modifiers().test(Wt::KeyboardModifier::Shift)) {
            updateSelection(old_pos, current_pos);
        }
        else {
            resetSelection();
        }

        reallocate_cursor = true;
        break;
    }

    default:
        return;
    } // ~switch

    update();

}

void CodeEditor::handleKeyUp(const Wt::WKeyEvent& e) {
    std::cout << "Up:" << e.key() << ":" << modifiersToString(e.modifiers()) << ":" << e.charCode() << ":" << e.text() << std::endl;
}

void CodeEditor::maximize_range(Scroll& scroll) {
    scroll.range_ = 0;

    if (scroll.direction == ScrollDirection::HORIZONTAL) {
        for (const TTextLine& el : txt_editor) {
            scroll.maximize_range(el.size());
        }
    }
    else { // VERTICAL
        scroll.maximize_range(txt_editor.size());
    }
}

std::string CodeEditor::getTextInRange(std::size_t row_start, std::size_t col_start, std::size_t row_end, std::size_t col_end) {
    Wt::WStringStream text;

    if (row_start == row_end) { // One row selected
        for (std::size_t col = col_start; col < col_end; col++) {
            text << txt_editor[row_start][col].UTF8;
        }
    }
    else { // More than one row selected
        for (std::size_t r = row_start; r <= row_end; r++) {
            if (r == row_start) { // We are in the first row
                for (std::size_t col = col_start; col < txt_editor[r].size(); col++) {
                    text << txt_editor[r][col].UTF8;
                }
            }
            else if (r == row_end) { // We are in the last row
                text << '\n';
                for (std::size_t col = 0; col < col_end; col++) {
                    text << txt_editor[r][col].UTF8;
                }
            }
            else { // We are between start and end rows
                text << '\n';
                for (std::size_t col = 0; col < txt_editor[r].size(); col++) {
                    text << txt_editor[r][col].UTF8;
                }
            }
        }
    }

    return text.str();
}

std::string CodeEditor::text() {
    return getTextInRange(0, 0,
        txt_editor.size()-1, txt_editor[txt_editor.size()-1].size());
}

std::string CodeEditor::selectedText() {
    return getTextInRange(selection.selection_start.row, selection.selection_start.col,
        selection.selection_end.row, selection.selection_end.col);
}

void CodeEditor::putToClipboard() {
    this->doJavaScript(
        "let base64_t = '" + Wt::Utils::base64Encode(Wt::Utils::urlEncode(selectedText()), false) + "'; \
        let ascii_t = decodeURIComponent(atob(base64_t)); \
            navigator.clipboard.writeText(ascii_t).then(function() { \
                /* clipboard successfully set */ \
            }, function() { \
                /* clipboard write failed */ \
                alert('Clipboard is not available!'); \
            });"
    );
}

void CodeEditor::getFromClipboard() {
    this->doJavaScript(
        "navigator.clipboard.readText().then(clipText => \
            Wt.emit('" + this->id() + "', 'update_from_clipboard', clipText) \
        );"
    );
}

EBackgroundStyle CodeEditor::setFlag(EBackgroundStyle base, EBackgroundStyle flag) {
    return static_cast<EBackgroundStyle>(static_cast<int>(base) | static_cast<int>(flag));
}

EBackgroundStyle CodeEditor::removeFlag(EBackgroundStyle base, EBackgroundStyle flag) {
    return static_cast<EBackgroundStyle>(static_cast<int>(base) ^ static_cast<int>(flag));
}

void CodeEditor::setSelection(EBackgroundStyle style, EBackgroundStyle oper(EBackgroundStyle, EBackgroundStyle)) {
    if (selection.is_selected) {
        CursorPos cur_pos = selection.selection_start;
        while (cur_pos != selection.selection_end) {
            if (cur_pos.row < txt_editor.size() and cur_pos.col < txt_editor[cur_pos.row].size()) {
                txt_editor[cur_pos.row][cur_pos.col].backgroundStyle = oper(txt_editor[cur_pos.row][cur_pos.col].backgroundStyle, style);
            }

            if (cur_pos.col == txt_editor[cur_pos.row].size()) {
                cur_pos.row++;
                cur_pos.col = 0;
            }
            else {
                cur_pos.col++;
            }
        }
    }
}

void CodeEditor::updateSelection(const CursorPos& old_pos, const CursorPos& new_pos) {
    setSelection(EBackgroundStyle::SELECTED, removeFlag);
    selection.select(old_pos, new_pos);
    setSelection(EBackgroundStyle::SELECTED, setFlag);
}

void CodeEditor::resetSelection() {
    setSelection(EBackgroundStyle::SELECTED, removeFlag);
    selection.reset();
}

void CodeEditor::deleteSelected() {
    CursorPos sel_start = selection.selection_start;
    CursorPos sel_end = selection.selection_end;
    resetSelection();

    if (sel_start.row == sel_end.row) { // One row selected
        txt_editor[sel_start.row].erase(txt_editor[sel_start.row].begin() + sel_start.col, txt_editor[sel_start.row].begin() + sel_end.col);
    }
    else { // More than one row selected
        std::size_t r = sel_end.row;
        bool done = false;
        do {
            if (r == sel_start.row) { // We are in the first row
                txt_editor[r].erase(txt_editor[r].begin() + sel_start.col, txt_editor[r].end());
            }
            else if (r == sel_end.row) { // We are in the last row
                txt_editor[r].erase(txt_editor[r].begin(), txt_editor[r].begin() + sel_end.col);
            }
            else { // We are between start and end rows
                txt_editor.erase(txt_editor.begin() + r); // Erase the entire line
            }

            if (r == sel_start.row) { // We're done
                // Merge the first and the last line
                txt_editor[r].insert(txt_editor[r].end(), txt_editor[r + 1].begin(), txt_editor[r + 1].end());
                txt_editor.erase(txt_editor.begin() + r + 1);

                done = true;
            }
            else {
                r--;
            }
        } while (!done);
    }

    // Adjust cursor position if needed
    if (current_pos > sel_start) {
        current_pos = sel_start;
    }
}
