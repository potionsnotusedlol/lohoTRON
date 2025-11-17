#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>
namespace Ui {
class MainMenuWidget;
}
class MainMenuWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenuWidget(QWidget *parent = nullptr);
    ~MainMenuWidget() override;

signals:
    void startGameRequested(int rounds, int bots);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    Ui::MainMenuWidget *ui;

    void updateSpacings();

private slots:
    void onStartButtonClicked();
};

#endif // MAINMENUWIDGET_H
