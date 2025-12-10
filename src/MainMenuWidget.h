#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenuWidget; }
QT_END_NAMESPACE

class MainMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget* parent = nullptr);
    ~MainMenuWidget();
protected:
    void resizeEvent(QResizeEvent* event);
private:
    Ui::MainMenuWidget *ui;

    void updateSpacings();
};

#endif // MAINMENUWIDGET_H