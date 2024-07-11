#include "selectdialog.h"
#include "ui_selectdialog.h"

#include "bookinfodialog.h"
#include "userinfodialog.h"

SelectDialog::SelectDialog(QWidget *parent, int _bookID, int _userID) :
    QDialog(parent),
    ui(new Ui::SelectDialog)
{
    ui->setupUi(this);

    // 设置输入框验证器
    ui->lineEdit->setValidator(new QIntValidator(0, INT_MAX, this));

    // 初始化模型和视图
    bookModel = new QStandardItemModel();
    userModel = new QStandardItemModel();
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 根据不同的初始化条件显示不同的数据表格
    if (_bookID != -1) {
        ui->lineEdit->setPlaceholderText(tr("用户编号..."));
        this->setWindowTitle(tr("选择用户"));
        displayUserData();
    } else if (_userID != -1) {
        ui->lineEdit->setPlaceholderText(tr("图书编号..."));
        this->setWindowTitle(tr("选择图书"));
        displayBookData();
    } else {
        // 其他情况处理
    }
}

SelectDialog::~SelectDialog()
{
    delete ui;
}

void SelectDialog::initBookTable()
{
    // 初始化图书表格视图
    bookModel->clear();
    bookModel->setColumnCount(4);
    bookModel->setHeaderData(0, Qt::Horizontal, tr("名称"));
    bookModel->setHeaderData(1, Qt::Horizontal, tr("编号"));
    bookModel->setHeaderData(2, Qt::Horizontal, tr("总数量"));
    bookModel->setHeaderData(3, Qt::Horizontal, tr("剩余数量"));
    ui->tableView->setModel(bookModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableView->setColumnWidth(1, 100);
    ui->tableView->setColumnWidth(2, 50);
    ui->tableView->setColumnWidth(3, 50);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
}

void SelectDialog::initUserTable()
{
    // 初始化用户表格视图
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

void SelectDialog::displayBookData()
{
    // 显示图书数据
    initBookTable();
    for (auto p = lib.books.begin(); p != lib.books.end(); p = p->next) {
        appendSingleBook(p);
    }
}

void SelectDialog::displayUserData()
{
    // 显示用户数据
    initUserTable();
    for (auto p = lib.users.begin(); p != lib.users.end(); p = p->next) {
        appendSingleUser(p);
    }
}

void SelectDialog::appendSingleBook(Node<BookInfo>* p)
{
    // 添加单本图书到表格
    if (!p) return;
    QList<QStandardItem*> list;
    list << new QStandardItem(p->elem.name.data())
         << new QStandardItem(QString::number(p->elem.identifier))
         << new QStandardItem(QString::number(p->elem.quantity))
         << new QStandardItem(QString::number(p->elem.quantity - p->elem.readers.size()));
    bookModel->appendRow(list);
}

void SelectDialog::appendSingleUser(Node<UserInfo>* p)
{
    // 添加单个用户到表格
    if (!p) return;
    QList<QStandardItem*> list;
    list << new QStandardItem(p->elem.name.data())
         << new QStandardItem(QString::number(p->elem.identifier))
         << new QStandardItem(QString::number(p->elem.books.size()));
    userModel->appendRow(list);
}

int SelectDialog::getSelection()
{
    // 获取当前选中行的编号
    int curRow = ui->tableView->currentIndex().row();
    QAbstractItemModel* itemModel = ui->tableView->model();
    QModelIndex itemIdx = itemModel->index(curRow, 1);
    QVariant dataTmp = itemModel->data(itemIdx);
    return dataTmp.toInt();
}

int SelectDialog::getSelection(const QModelIndex &index)
{
    // 重载的获取选中行编号函数
    int curRow = index.row();
    QAbstractItemModel* itemModel = ui->tableView->model();
    QModelIndex itemIdx = itemModel->index(curRow, 1);
    QVariant dataTmp = itemModel->data(itemIdx);
    return dataTmp.toInt();
}

void SelectDialog::on_tableView_clicked(const QModelIndex &index)
{
    // 表格单击事件处理
    ui->lineEdit->setText(QString::number(getSelection(index)));
}

void SelectDialog::on_buttonBox_accepted()
{
    // 确认按钮点击事件处理，发送选中编号并接受对话框
    emit sendData(ui->lineEdit->text());
    accept();
}

void SelectDialog::on_tableView_doubleClicked()
{
    // 表格双击事件处理，发送选中编号并接受对话框
    emit sendData(ui->lineEdit->text());
    accept();
}
