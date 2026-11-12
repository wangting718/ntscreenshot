#include "TextEdit.h"
#include <QTextDocument>
#include <QTextFormat>
#include <QKeyEvent>
#include <QTextCursor>
#include <QWheelEvent>
#include <QtMath>

const int PADDING = 6;
const int BASE_WIDTH = 50;
const int BASE_HEIGHT = 26;

int TextEdit::adjustedFontSize(int currentPixelSize, int wheelSteps)
{
    return qBound(kMinFontPixelSize,
                  currentPixelSize + wheelSteps * kWheelStepPixelSize,
                  kMaxFontPixelSize);
}

TextEdit::TextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setAcceptDrops(true);
    this->setAcceptRichText(false);
    connect(this, &QTextEdit::textChanged, this, &TextEdit::onTextChanged);
    this->setFixedSize(BASE_WIDTH, BASE_HEIGHT);
    this->setLineWrapMode(QTextEdit::NoWrap);
    this->document()->setDocumentMargin(PADDING);
}

void TextEdit::onTextChanged()
{
    // Document dimensions already include the same padding used to lay out
    // the caret and text, including an empty line and mixed-font input.
    const int w = qCeil(document()->idealWidth()) + 2 * frameWidth() + cursorWidth();
    const int h = qCeil(document()->size().height()) + 2 * frameWidth();
    this->setFixedWidth(qMax(w, BASE_WIDTH));
    this->setFixedHeight(qMax(h, BASE_HEIGHT));
}

QPoint TextEdit::startCursorPoint()
{
    const QTextCursor first(document());
    return viewport()->mapTo(parentWidget(), cursorRect(first).topLeft());
}

void TextEdit::setStyle(const QFont &font, const QColor &color)
{
    color_ = color;
    const QString fontSize = font.pixelSize() > 0
        ? QStringLiteral("%1px").arg(font.pixelSize())
        : QStringLiteral("%1pt").arg(font.pointSizeF());

    this->setStyleSheet(QStringLiteral(
        "background:transparent;"
        "color:%1;"
        "border:1px dotted %2;"
        "padding:0px;"
        "font-size:%3;")
        .arg(color.name(), color.name(), fontSize));
    this->setFont(font);
    this->document()->setDefaultFont(font);
    this->setCurrentFont(font);
    this->ensurePolished();
    onTextChanged();
}

int TextEdit::currentFontPixelSize() const
{
    const int pixelSize = font().pixelSize();
    if (pixelSize > 0) {
        return pixelSize;
    }
    // 未设 pixelSize 时用 pointSize 换算兜底。
    return qMax(1, qRound(font().pointSizeF()));
}

void TextEdit::setFontPixelSize(int pixelSize)
{
    pixelSize = qBound(kMinFontPixelSize, pixelSize, kMaxFontPixelSize);
    if (pixelSize == currentFontPixelSize()) {
        return;
    }

    QFont newFont = font();
    newFont.setPixelSize(pixelSize);

    // 已输入的文字也要跟着变大/变小，不能只改后续输入的字体。
    // 先全选合并字符格式，再恢复光标位置。
    const QTextCursor savedCursor = textCursor();
    QTextCursor all(document());
    all.select(QTextCursor::Document);
    QTextCharFormat format;
    format.setFontPixelSize(pixelSize);
    all.mergeCharFormat(format);
    setTextCursor(savedCursor);

    setStyle(newFont, color_);
    emit fontSizeChanged(pixelSize);
}

void TextEdit::adjustFontSize(int wheelSteps)
{
    if (wheelSteps == 0) {
        return;
    }
    setFontPixelSize(adjustedFontSize(currentFontPixelSize(), wheelSteps));
}

void TextEdit::wheelEvent(QWheelEvent *e)
{
    // 高精度滚轮/触控板一次可能只发不足一格的增量，攒够 120 再算一格，
    // 不足一格的余量留到下次，避免小幅滚动被丢掉。
    wheelRemainder_ += e->angleDelta().y();
    const int steps = wheelRemainder_ / 120;
    wheelRemainder_ -= steps * 120;
    if (steps != 0) {
        adjustFontSize(steps);
    }
    e->accept();
}

void TextEdit::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        emit cancelRequested();
        e->accept();
        return;
    }
    if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        && e->modifiers().testFlag(Qt::ControlModifier)) {
        emit commitRequested();
        e->accept();
        return;
    }
    QTextEdit::keyPressEvent(e);
}
