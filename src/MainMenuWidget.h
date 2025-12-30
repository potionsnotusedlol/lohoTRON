#ifndef MAINMENUWIDGET_H
#define MAINMENUWIDGET_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <cstdlib>
#include <ctime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainMenuWidget; }
QT_END_NAMESPACE

class MainMenuWidget : public QWidget {
    Q_OBJECT
public:
    explicit MainMenuWidget(QWidget* parent = nullptr);
    ~MainMenuWidget();
    QMediaPlayer* music() const;
protected:
    void resizeEvent(QResizeEvent* event);
private:
    Ui::MainMenuWidget *ui;
    QMediaPlayer *music_player;
    QAudioOutput *music_output;

    void updateSpacings();
};

#endif // MAINMENUWIDGET_H