#pragma once

#include <QWidget>
#include <QList>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QShowEvent;
class DrawSettings : public QWidget
{
    Q_OBJECT
public:
    DrawSettings(QWidget *parent = nullptr);
    static int penWidth();
    static int fontSize();
    static QColor currentColor();
    // 滚轮调字号时同步静态字号与下拉框，但不发 sigChanged。
    // sigChanged 会走到 DrawPanel::onSettingChanged -> Drawer::setMode，
    // 而 setMode 会先 saveText() 提交并关闭正在编辑的输入框，所以
    // 编辑中的字号同步必须走这条静默路径；Drawer 已直接改好了
    // 待提交文字的 DrawMode 字体，这里只负责记下来、显示出来。
    void setFontSizeSilently(int pixelSize);

    // 按当前屏幕 DPI 重新应用所有尺寸/样式（构造函数与 DPI 变化时调用）
    void rescaleForDpi();

signals:
    void sigChanged(int penWidth, int fontSize, QColor color);

public slots:
    void onFontSizeChanged(const QString &text);
    void onCurrentColor();
    void onColor();
    void onPenWidthSelected();

protected:
    void showEvent(QShowEvent *event) override;

private:
    void refreshCurrentColorButton();
    void syncPresetSelection();

    QLabel *penWidthLabel_ = nullptr;
    QLabel *colorLabel_ = nullptr;
    QComboBox *sizeList_ = nullptr;
    QPushButton *pbCurrentColor_ = nullptr;
    QList<QPushButton*> penWidthBtns_;
    QList<QPushButton*> colorBtns_;
    QHBoxLayout *row1_ = nullptr;
};
