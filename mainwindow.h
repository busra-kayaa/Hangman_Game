#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include "label.h"
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int secilenId;
    int yanlisTahminSayisi,dogruTahminSayisi;
    QString tahminHarf;
    QString selectedWord;

    QList<QLabel*> hangmanParts; // Adam parçalarının listesi

    void listele();
    void resetGame();
    void initializeHangmanParts(); // Adam parçalarını başlatma fonksiyonu
    void updateHangmanImage(); // Adam görüntüsünü güncelleme fonksiyonu

private slots:
    void on_Ekle_clicked();
    void on_Degistir_clicked();
    void on_Sil_clicked();
    void on_KelimeView_clicked(const QModelIndex &index);
    void on_Basla_clicked();
    void on_Tahmin_clicked();

private:
    Ui::MainWindow *ui;

    QSqlQuery *sorgu, *eklemeSorgu, *guncelleSorgu, *silmeSorgu;
    QSqlQueryModel *model;

    QStringList currentWordLabels;

    void setupLineEditsForWord(const QString &word);
    bool oyunBasladi;

};
#endif // MAINWINDOW_H
