#pragma once

#include <QColor>
#include <QTextEdit>

class QWheelEvent;

class TextEdit : public QTextEdit
{
    Q_OBJECT

public:
    // Snipaste 式滚轮调字号的步长与范围（与 Drawer 共用，见 adjustedFontSize）。
    static constexpr int kMinFontPixelSize = 8;
    static constexpr int kMaxFontPixelSize = 72;
    static constexpr int kWheelStepPixelSize = 2;

    // wheelSteps：滚轮格数（上滚为正）。返回钳制后的新字号。
    static int adjustedFontSize(int currentPixelSize, int wheelSteps);

    TextEdit(QWidget *parent);
    QPoint startCursorPoint();
    void setStyle(const QFont &font, const QColor &color);
    int currentFontPixelSize() const;
    // 把已输入的文字和后续输入都改到 pixelSize，并重算输入框大小。
    void setFontPixelSize(int pixelSize);
    void adjustFontSize(int wheelSteps);

signals:
    void commitRequested();
    void cancelRequested();
    // 字号被滚轮改动后发出，Drawer 用它同步待提交的 DrawMode 与工具栏。
    void fontSizeChanged(int pixelSize);

public slots:
    void onTextChanged();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    // 滚轮调字号（Snipaste 行为）：上滚变大、下滚变小；不滚动内容。
    void wheelEvent(QWheelEvent *e) override;

private:
    QColor color_ = QColor("#FF0000");
    int wheelRemainder_ = 0;
};
