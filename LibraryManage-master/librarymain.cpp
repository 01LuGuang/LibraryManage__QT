#include "librarymain.h"
#include "ui_librarymain.h"
#include "bookinfodialog.h"
#include "userinfodialog.h"

#include <QTableView>
#include <QMessageBox>
#include <QFileDialog>

LibraryMain::LibraryMain(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LibraryMain)
{
    ui->setupUi(this);

    // 隐藏翻页按钮和图书/用户切换按钮
    ui->pageUpButton->setHidden(true);
    ui->pageDnButton->setHidden(true);
    ui->bookSwitchButton->setDisabled(true);

    // 初始化图书和用户的数据模型
    bookModel = new QStandardItemModel();
    userModel = new QStandardItemModel();

    // 设置表格选择行为和选择模式
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 显示图书数据
    displayBookData();

    // 如果未登录用户，则返回
    if (loginUserID == -1) {
        return;
    }

    // 根据登录用户是否为管理员来显示或隐藏按钮
    if (!lib.isAdmin(lib.findUser(loginUserID))) {
        isLoginAdmin = false;
        ui->addButton->setHidden(true);
    } else {
        isLoginAdmin = true;
    }
}

LibraryMain::~LibraryMain()
{
    delete ui;
}

void LibraryMain::closeEvent(QCloseEvent *event)
{
    // 弹出对话框询问是否保存并退出
    int ret = QMessageBox::question(this, tr("图书管理"), tr("确认要保存并退出吗？"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        // 保存图书和用户数据到文件
        if (lib.writeBook(lib.bookPath) || lib.writeUser(lib.userPath)) {
            QMessageBox::warning(this, tr("错误"), tr("写入文件失败。"), QMessageBox::Ok);
            return;
        }
        event->accept();
        break;
    case QMessageBox::Discard:
        event->accept();
        break;
    case QMessageBox::Cancel:
    default:
        event->ignore();
        break;
    }
}

void LibraryMain::initBookTable()
{
    // 禁用按钮并清空图书模型
    disableButton();
    bookModel->clear();
    bookModel->setColumnCount(4);
    bookModel->setHeaderData(0, Qt::Horizontal, tr("名称"));
    bookModel->setHeaderData(1, Qt::Horizontal, tr("编号"));
    bookModel->setHeaderData(2, Qt::Horizontal, tr("总数量"));
    bookModel->setHeaderData(3, Qt::Horizontal, tr("剩余数量"));
    ui->tableView->setModel(bookModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 根据窗口大小设置列的固定宽度和可伸缩模式
    if (this->size().width() >= 300) {
        ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
        ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
        ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
        ui->tableView->setColumnWidth(1, 125);
        ui->tableView->setColumnWidth(2, 75);
        ui->tableView->setColumnWidth(3, 75);
    }

    // 设置表格属性：禁止编辑、交替行颜色
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
}

void LibraryMain::initUserTable()
{
    // 禁用按钮并清空用户模型
    disableButton();
    userModel->clear();
    userModel->setColumnCount(3);
    userModel->setHeaderData(0, Qt::Horizontal, tr("用户名"));
    userModel->setHeaderData(1, Qt::Horizontal, tr("编号"));
    userModel->setHeaderData(2, Qt::Horizontal, tr("已借阅数量"));
    ui->tableView->setModel(userModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableView->setColumnWidth(2, 75);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
}

void LibraryMain::displayBookData()
{
    // 初始化图书表格并显示所有图书数据
    initBookTable();
    for (auto p = lib.books.begin(); p != lib.books.end(); p = p->next) {
        appendSingleBook(p);
    }
}

void LibraryMain::displayUserData()
{
    // 初始化用户表格并显示所有用户数据
    initUserTable();
    for (auto p = lib.users.begin(); p != lib.users.end(); p = p->next) {
        appendSingleUser(p);
    }
}

void LibraryMain::on_bookSwitchButton_clicked()
{
    // 切换到图书搜索模式
    ui->bookSwitchButton->setDisabled(true);
    ui->bookSwitchButton->setChecked(true);
    ui->userSwitchButton->setDisabled(false);
    ui->userSwitchButton->setChecked(false);
    ui->borrowButton->setDisabled(true);
    ui->returnButton->setDisabled(true);
    ui->editButton->setDisabled(true);
    ui->searchBox->setPlaceholderText(tr("搜索图书..."));
    ui->searchButton->click();
}

void LibraryMain::on_userSwitchButton_clicked()
{
    // 切换到用户搜索模式
    ui->userSwitchButton->setDisabled(true);
    ui->userSwitchButton->setChecked(true);
    ui->bookSwitchButton->setDisabled(false);
    ui->bookSwitchButton->setChecked(false);
    ui->borrowButton->setDisabled(true);
    ui->returnButton->setDisabled(true);
    ui->editButton->setDisabled(true);
    ui->searchBox->setPlaceholderText(tr("搜索用户..."));
    ui->searchButton->click();
}

void LibraryMain::displayBookList(List<Node<BookInfo>*> list)
{
    // 显示指定的图书列表
    initBookTable();
    for (auto p = list.begin(); p != list.end(); p = p->next) {
        appendSingleBook(p->elem);
    }
}

void LibraryMain::displayUserList(List<Node<UserInfo>*> list)
{
    // 显示指定的用户列表
    initUserTable();
    for (auto p = list.begin(); p != list.end(); p = p->next) {
        appendSingleUser(p->elem);
    }
}

void LibraryMain::appendSingleBook(Node<BookInfo>* p)
{
    // 向图书表格添加单个图书条目
    if (!p) return;
    QList<QStandardItem*> list;
    list << new QStandardItem(p->elem.name.data())
         << new QStandardItem(std::to_string(p->elem.identifier).data())
         << new QStandardItem(std::to_string(p->elem.quantity).data())
         << new QStandardItem(std::to_string(p->elem.quantity - p->elem.readers.size()).data());
    bookModel->appendRow(list);
}

void LibraryMain::appendSingleUser(Node<UserInfo>* p)
{
    // 向用户表格添加单个用户条目
    if (!p) return;
    QList<QStandardItem*> list;
    list << new QStandardItem(p->elem.name.data())
         << new QStandardItem(std::to_string(p->elem.identifier).data())
         << new QStandardItem(std::to_string(p->elem.books.size()).data());
    userModel->appendRow(list);
}

void LibraryMain::displaySingleBook(Node<BookInfo>* p)
{
    // 显示单个图书详情
    initBookTable();
    appendSingleBook(p);
}

void LibraryMain::displaySingleUser(Node<UserInfo>* p)
{
    // 显示单个用户详情
    initUserTable();
    appendSingleUser(p);
}

void LibraryMain::on_searchButton_clicked()
{
    // 根据搜索框内容进行图书或用户搜索
    QString query = ui->searchBox->text();
    if (!ui->bookSwitchButton->isEnabled()) {
        if (query.isEmpty()) {
            displayBookData();
            return;
        }
        if (ui->selectNameButton->isChecked()) {
            displayBookList(lib.fuzzyFindBook(query.toStdString()));
        } else {
            displaySingleBook(lib.findBook(query.toInt()));
        }
    } else {
        if (query.isEmpty()) {
            displayUserData();
            return;
        }
        if (ui->selectNameButton->isChecked()) {
            displayUserList(lib.fuzzyFindUser(query.toStdString()));
        } else {
            displaySingleUser(lib.findUser(query.toInt()));
        }
    }
}



int LibraryMain::getSelection(const QModelIndex &index) {
    if (!index.isValid()) {
        return -1;
    }
    int curRow = index.row();
    QAbstractItemModel* itemModel = ui->tableView->model();
    QModelIndex itemIdx = itemModel->index(curRow, 1);
    QVariant dataTmp = itemModel->data(itemIdx);
    return dataTmp.toInt();
}

void LibraryMain::on_borrowButton_clicked() {
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请选择一本书。"), QMessageBox::Ok);
        return;
    }
    int bookID = getSelection(selectedIndexes.first());
    if (lib.borrowBook(loginUserID, bookID)) {
        QMessageBox::information(this, tr("提示"), tr("没有这本书剩余了。"), QMessageBox::Ok);
        return;
    }
    displayBookData();
    ui->statusbar->showMessage(tr("成功借阅《") + tr(lib.findBook(bookID)->elem.name.data())
                               + tr("》。"), 3000);
}

void LibraryMain::on_returnButton_clicked() {
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请选择一本书。"), QMessageBox::Ok);
        return;
    }
    int bookID = getSelection(selectedIndexes.first());
    if (lib.returnBook(loginUserID, bookID)) {
        QMessageBox::information(this, tr("提示"), tr("你没有借阅这本书。"), QMessageBox::Ok);
        return;
    }
    displayBookData();
    ui->statusbar->showMessage(tr("成功归还《") + tr(lib.findBook(bookID)->elem.name.data())
                               + tr("》。"), 3000);
}


void LibraryMain::on_tableView_clicked(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    if (!ui->bookSwitchButton->isEnabled()) {
        int bookID = getSelection(index);
        updateButton(bookID, loginUserID);
    } else {
        ui->editButton->setDisabled(false);
    }
}


void LibraryMain::updateButton(int bookID, int userID) {
    auto book = lib.findBook(bookID);
    ui->editButton->setDisabled(false);

    if (userID == -1) {
        ui->borrowButton->setDisabled(true);
        ui->returnButton->setDisabled(true);
        return;
    }

    if (book->elem.quantity == book->elem.readers.size()) {
        ui->borrowButton->setDisabled(true);
    } else {
        ui->borrowButton->setDisabled(false);
    }

    if (lib.findUser(userID)->elem.books.find(book)) {
        ui->returnButton->setDisabled(false);
    } else {
        ui->returnButton->setDisabled(true);
    }
}

void LibraryMain::disableButton() {
    ui->borrowButton->setDisabled(true);
    ui->returnButton->setDisabled(true);
    ui->editButton->setDisabled(true);
}

void LibraryMain::on_editButton_clicked() {
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请选择一条记录。"), QMessageBox::Ok);
        return;
    }

    int selectedID = getSelection(selectedIndexes.first());

    if (!ui->bookSwitchButton->isEnabled()) {
        BookInfoDialog bookDialog(this, selectedID);
        bookDialog.exec();
    } else {
        UserInfoDialog userDialog(this, selectedID);
        userDialog.exec();
    }
    ui->searchButton->click();
}


void LibraryMain::on_addButton_clicked() {
    QDialog *dialog = nullptr;
    if (!ui->bookSwitchButton->isEnabled()) {
        dialog = new BookInfoDialog(this);
    } else {
        dialog = new UserInfoDialog(this);
    }

    if (dialog) {
        dialog->exec();
        delete dialog;
    }

    ui->searchButton->click();
}


void LibraryMain::on_tableView_doubleClicked() {
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        return;
    }

    ui->editButton->click();
}



void LibraryMain::on_writeDataAction_triggered() {
    bool bookState = lib.writeBook(lib.bookPath);
    bool userState = lib.writeUser(lib.userPath);
    if (!bookState || !userState) {
        QMessageBox::warning(this, tr("错误"), tr("写入文件失败。"), QMessageBox::Ok);
        return;
    }
    ui->statusbar->showMessage(tr("成功写入文件 ") + tr(lib.bookPath)
                               + tr(", ") + tr(lib.userPath), 3000);
}


void LibraryMain::on_aboutMeAction_triggered() {
    if (loginUserID != -1) {
        UserInfoDialog userDialog(this, loginUserID);
        userDialog.exec();
    } else {
        QMessageBox::information(this, tr("提示"), tr("请先登录。"), QMessageBox::Ok);
    }
}


void LibraryMain::on_signOutAction_triggered() {
    // 清理登录状态
    loginUserID = -1;
    // 关闭主窗口
    close();
}

void LibraryMain::on_importAction_triggered() {
    QString bookFile = QFileDialog::getOpenFileName(this,
                tr("导入图书数据文件"), "./", tr("csv 文件 (*.csv)"));
    QString userFile = QFileDialog::getOpenFileName(this,
                tr("导入用户数据文件"), "./", tr("csv 文件 (*.csv)"));
    if (bookFile.isEmpty() || userFile.isEmpty()) return;
    if (!lib.read(userFile.toLatin1(), bookFile.toLatin1())) {
        QMessageBox::warning(this, tr("错误"), tr("读取文件失败。"), QMessageBox::Ok);
        return;
    }
    ui->searchButton->click();
}


void LibraryMain::on_exportAction_triggered() {
    QString bookFile = QFileDialog::getSaveFileName(this,
                tr("导出图书数据文件"), "./book.csv", tr("csv 文件 (*.csv)"));
    QString userFile = QFileDialog::getSaveFileName(this,
                tr("导出用户数据文件"), "./user.csv", tr("csv 文件 (*.csv)"));
    if (bookFile.isEmpty() || userFile.isEmpty()) return;
    if (!lib.writeBook(bookFile.toLatin1()) || !lib.writeUser(userFile.toLatin1())) {
        QMessageBox::warning(this, tr("错误"), tr("写入文件失败。"), QMessageBox::Ok);
        return;
    }
}


void LibraryMain::on_readDataAction_triggered() {
    if (QString(lib.bookPath).isEmpty() || QString(lib.userPath).isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("文件路径不能为空。"), QMessageBox::Ok);
        return;
    }

    if (!lib.read(lib.userPath, lib.bookPath)) {
        QMessageBox::warning(this, tr("错误"), tr("读取文件失败。"), QMessageBox::Ok);
        return;
    }
    ui->statusbar->showMessage(tr("成功读取文件 ") + tr(lib.bookPath)
                               + tr(", ") + tr(lib.userPath), 3000);
    ui->searchButton->click();
}


void LibraryMain::on_aboutAction_triggered() {
    QMessageBox aboutBox;
    aboutBox.setWindowTitle(tr("关于"));
    aboutBox.setText("hawa130的图书管理系统");
    aboutBox.setInformativeText(tr("一个易于使用的简单图书管理系统。\n\n项目地址：<a href=\"https://github.com/hawa130/LibraryManage\">https://github.com/hawa130/LibraryManage</a>"));
    aboutBox.exec();
}


