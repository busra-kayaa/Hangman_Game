#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QPixmap>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    oyunBasladi = false;
    yanlisTahminSayisi = 0;
    dogruTahminSayisi = 0;

    initializeHangmanParts(); // Adam parçalarını başlat

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("C:/Users/Busra/hangman.db");

    if(!db.open()){
        QMessageBox::critical(this, "Bağlantı Hatası", db.lastError().text());
    }
    sorgu = new QSqlQuery(db);
    eklemeSorgu = new QSqlQuery(db);
    guncelleSorgu = new QSqlQuery(db);
    silmeSorgu = new QSqlQuery(db);
    model = new QSqlQueryModel();

    listele();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listele()
{
    bool kontrol = sorgu->exec("select * from kelimeler");
    if(!kontrol) {
        QMessageBox::critical(this, "Sorgu Hatası", sorgu->lastError().text());
    }
    model->setQuery(*sorgu);
    ui->KelimeView->setModel(model);
}

void MainWindow::on_Ekle_clicked()
{
    if (ui->Kelime->text().isEmpty()) {
         QMessageBox::warning(this, "Hata","Lütfen bir kelime girin!");
         return;
    }

    eklemeSorgu->prepare("insert into kelimeler (kelime) values(?)");
    eklemeSorgu->addBindValue(ui->Kelime->text());

    if(!eklemeSorgu->exec()) {
        QMessageBox::critical(this, "Sorgu Hatası",eklemeSorgu->lastError().text());
    }

    listele();
}

void MainWindow::on_Degistir_clicked()
{
    guncelleSorgu->prepare("update kelimeler set kelime = ? where kelimeNo = ?");

    guncelleSorgu->addBindValue(ui->Kelime->text());
    guncelleSorgu->addBindValue(secilenId);

    if(!guncelleSorgu->exec()) {
        QMessageBox::critical(this, "Guncelleme Hatası", guncelleSorgu->lastError().text());
    }
    listele();
}

void MainWindow::on_Sil_clicked()
{
    silmeSorgu->prepare("delete from kelimeler where kelimeNo = ?");

    silmeSorgu->addBindValue(secilenId);

    if (!silmeSorgu->exec()) {
        QMessageBox::critical(this, "Silme Hatası",silmeSorgu->lastError().text());
    }
    listele();
}

void MainWindow::on_KelimeView_clicked(const QModelIndex &index)
{
    secilenId = model->index(index.row(), 0).data().toInt();
    ui->Kelime->setText(model->index(index.row(),1).data().toString());
}

void MainWindow::on_Basla_clicked()
{
    if (sorgu->exec("select kelime from kelimeler")) {
        QStringList kelimeList;
        while (sorgu->next()) {
            kelimeList.append(sorgu->value(0).toString());
        }
        int index = qrand() % kelimeList.size();
        selectedWord = kelimeList.at(index);

        qDebug() << "Seçilen kelime: " << selectedWord;
        setupLineEditsForWord(selectedWord);

        oyunBasladi = true; // Oyun başlatıldı
    }
}

void MainWindow::setupLineEditsForWord(const QString &word)
{
    int xSpacing = 35;
    int yPosition = 500;

    // Önce eski QLabel'leri sil
    foreach (const QString &label, currentWordLabels) {
        QLabel *existingLabel = ui->centralwidget->findChild<QLabel *>(label);
        if (existingLabel) {
            delete existingLabel;
        }
    }

    currentWordLabels.clear();

    // Yeni QLabel'leri oluştur ve yerleştir
    for (int i = 0; i < word.length(); ++i) {
        QLabel *lineEdit = new QLabel("_", ui->centralwidget);
        lineEdit->setObjectName(QString::number(i));  // QLabel'e benzersiz isim atama
        lineEdit->setStyleSheet("font: bold 24px Arial; color: black;");
        lineEdit->resize(30, 50);
        lineEdit->move(8 * xSpacing + i * xSpacing, yPosition);
        lineEdit->show();

        currentWordLabels.append(lineEdit->objectName());
    }
}

void MainWindow::on_Tahmin_clicked()
{
    if (!oyunBasladi) {
        QMessageBox::warning(this, "Hata", "Oyuna başlamadan harf tahmini yapamazsınız! Lütfen önce Başla butonuna tıklayın.");
        return;
    }

    tahminHarf = ui->Harf->text().toUpper();
    if (tahminHarf.isEmpty() || tahminHarf.length() > 1) {
        QMessageBox::warning(this, "Hata", "Lütfen bir tane harf girin!");
        return;
    }

    // Girilen karakter sayı mı?
    if (tahminHarf[0].isDigit()) {
        QMessageBox::warning(this, "Hata", "Sayı tahmin edemezsiniz! Lütfen bir harf girin.");
        return;
    }

    bool harfBulundu = false;

    for (int i = 0; i < selectedWord.length(); ++i) {
           if (selectedWord[i].toUpper() == tahminHarf) {
               QLabel *label = ui->centralwidget->findChild<QLabel *>(QString::number(i));
               if (label && label->text() == "_") { // Sadece boş olan yerlere yaz
                   label->setText(tahminHarf);
                   harfBulundu = true;
                   dogruTahminSayisi++; // Doğru tahmin sayısını artır
               }
           }
       }

    if (!harfBulundu) {
        ui->listWidget->addItem(tahminHarf);
        yanlisTahminSayisi++;

        updateHangmanImage();
    }

    // Kelime tamamen doğru tahmin edildiyse
        if (dogruTahminSayisi == selectedWord.length()) {
            QMessageBox::information(this, "Tebrikler!", "Kelimeyi doğru tahmin ettiniz!");
            resetGame();
            return;
        }
    ui->Harf->clear();
}

void MainWindow::updateHangmanImage()
{
    if (yanlisTahminSayisi > 0 && yanlisTahminSayisi <= hangmanParts.size()) {
        hangmanParts[yanlisTahminSayisi - 1]->setVisible(true);
    }

    if (yanlisTahminSayisi == hangmanParts.size()) {
        QMessageBox::critical(this, "Oyun Bitti", "Adam Asıldı! Oyunu kaybettiniz.");
        resetGame();
    }
}

void MainWindow::initializeHangmanParts()
{
    // Adamın parçalarını oluştur ve yerleştir
    QLabel *horizontal_line = new QLabel(this);
    horizontal_line->setPixmap(QPixmap(":/res/images/yatayCizgi.png"));
    horizontal_line->setGeometry(330,391, 151, 20);
    horizontal_line->setVisible(false); // Başlangıçta görünmez

    QLabel *vertical_line = new QLabel(this);
    vertical_line->setPixmap(QPixmap(":/res/images/dikCizgi.png"));
    vertical_line->setGeometry(400,20, 20, 381);
    vertical_line->setVisible(false);

    QLabel *horizontal_line2 = new QLabel(this);
    horizontal_line2->setPixmap(QPixmap(":/res/images/yatayCizgi.png"));
    horizontal_line2->setGeometry(398,10, 119, 21);
    horizontal_line2->setVisible(false);

    QLabel *rope = new QLabel(this);
    rope->setPixmap(QPixmap(":/res/images/dikCizgi.png"));
    rope->setGeometry(510,21, 20, 61);
    rope->setVisible(false);

    QLabel *head = new QLabel(this);
    head->setPixmap(QPixmap(":/res/images/cember.png"));
    head->setScaledContents(true);
    head->setGeometry(480, 80, 71, 61);
    head->setVisible(false);

    QLabel *body = new QLabel(this);
    body->setPixmap(QPixmap(":/res/images/dikCizgi.png"));
    body->setGeometry(510, 138, 20, 152);
    body->setVisible(false);

    QLabel *leftArm = new QLabel(this);
    leftArm->setPixmap(QPixmap(":/res/images/sol.png"));
    leftArm->setScaledContents(true);
    leftArm->setGeometry(468, 170, 48, 40);
    leftArm->setVisible(false);

    QLabel *rightArm = new QLabel(this);
    rightArm->setPixmap(QPixmap(":/res/images/sag.png"));
    rightArm->setScaledContents(true);
    rightArm->setGeometry(511, 170, 47, 40);
    rightArm->setVisible(false);

    QLabel *leftLeg = new QLabel(this);
    leftLeg->setPixmap(QPixmap(":/res/images/sol.png"));
    leftLeg->setScaledContents(true);
    leftLeg->setGeometry(460, 290, 51, 41);
    leftLeg->setVisible(false);

    QLabel *rightLeg = new QLabel(this);
    rightLeg->setPixmap(QPixmap(":/res/images/sag.png"));
    rightLeg->setScaledContents(true);
    rightLeg->setGeometry(518, 289, 42, 42);
    rightLeg->setVisible(false);

    // Parçaları bir listeye ekle
    hangmanParts = {horizontal_line, vertical_line, horizontal_line2, rope, head, body, leftArm, rightArm,  leftLeg, rightLeg};
}

void MainWindow::resetGame()
{
    yanlisTahminSayisi = 0;
    dogruTahminSayisi = 0;
    ui->listWidget->clear();
    ui->Harf->clear();
    oyunBasladi = false;

    // Tüm parçaları görünmez yap
    foreach (QLabel *part, hangmanParts) {
        part->setVisible(false);
    }

    foreach (const QString &label, currentWordLabels) {
        QLabel *existingLabel = ui->centralwidget->findChild<QLabel *>(label);
        if (existingLabel) {
            delete existingLabel;
        }
    }
}
