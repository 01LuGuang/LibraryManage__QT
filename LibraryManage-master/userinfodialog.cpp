#include "userinfodialog.h"
#include "ui_userinfodialog.h"
#include "selectdialog.h"
#include "passworddialog.h"
#include "bookinfodialog.h"

#include <QMessageBox>
#include <QIntValidator>

UserInfoDialog::UserInfoDialog(QWidget *parent, int _userID) :
    QDialog(parent),
    ui(new Ui::UserInfoDialog) {
    ui->setupUi(this);

    bookModel = new QStandardItemModel();

    // 设置ID输入框的验证器，只能输入非负整数
    ui->idEdit->setValidator(new QIntValidator(0, INT_MAX, this));

    // 设置表格视图的选择行为
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    user = lib.findUser(_userID);
    if (user) {
        // 如果找到了用户，显示用户信息和借阅图书列表
        displayTable();
        ui->nameEdit->setText(tr(user->elem.name.data()));
        ui->idEdit->setText(QString::number(user->elem.identifier));
        ui->adminBox->setChecked(user->elem.type);
    } else {
        // 如果未找到用户，禁用相关按钮
        ui->deleteButton->setDisabled(true);
        ui->borrowButton->setDisabled(true);
        ui->returnButton->setDisabled(true);
        ui->pwdButton->setDisabled(true);
        ui->pwdButton->setToolTip(tr("保存用户后以修改密码"));
    }

    // 根据登录用户权限设置界面元素的可见性和可用性
    if (!isLoginAdmin && _userID != loginUserID) {
        ui->borrowButton->setHidden(true);
        ui->returnButton->setHidden(true);
        ui->deleteButton->setHidden(true);
        ui->buttonBox->setHidden(true);
        ui->nameEdit->setDisabled(true);
        ui->pwdButton->setHidden(true);
    }
    if (!isLoginAdmin) {
        ui->adminBox->setDisabled(true);
        ui->idEdit->setDisabled(true);
    }
}

UserInfoDialog::~UserInfoDialog() {
    delete ui;
}

void UserInfoDialog::receiveBookData(QString data) {
    auto book = lib.findBook(data.toInt());
    if (lib.borrowBook(user, book)) {
        QMessageBox::information(this, tr("提示"), tr("这本书已经没有剩余了。"), QMessageBox::Ok);
        return;
    }
    displayTable();
}

void UserInfoDialog::receivePwdData(QString data) {
    if (user) {
        user->elem.password = data.toStdString();
    } else {
        updateUserInfo();
        user->elem.password = data.toStdString();
    }
}

void UserInfoDialog::initBookTable() {
    ui->returnButton->setDisabled(true);
    bookModel->clear();
    bookModel->setColumnCount(4);
    bookModel->setHeaderData(0, Qt::Horizontal, tr("名称"));
    bookModel->setHeaderData(1, Qt::Horizontal, tr("编号"));
    bookModel->setHeaderData(2, Qt::Horizontal, tr("总数量"));
    bookModel->setHeaderData(3, Qt::Horizontal, tr("剩余数量"));
    ui->tableView->setModel(bookModel);

    // 设置表格列宽和布局
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableView->setColumnWidth(1, 120);
    ui->tableView->setColumnWidth(2, 60);
    ui->tableView->setColumnWidth(3, 60);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
}

void UserInfoDialog::displayTable() {
    initBookTable();
    ui->numLabel->setText(tr("已借阅 ") + QString::number(user->elem.books.size()) + tr(" 本"));
    for (auto p = user->elem.books.begin(); p != user->elem.books.end(); p = p->next) {
        appendSingleBook(p->elem);
    }
}

// 添加单本书籍到表格中
void UserInfoDialog::appendSingleBook(Node<BookInfo>* p) {
    if (!p) return;
    QList<QStandardItem*> list;
    list << new QStandardItem(QString::fromStdString(p->elem.name))
         << new QStandardItem(QString::number(p->elem.identifier))
         << new QStandardItem(QString::number(p->elem.quantity))
         << new QStandardItem(QString::number(p->elem.quantity - p->elem.readers.size()));
    bookModel->appendRow(list);
}

// 处理归还按钮点击事件
void UserInfoDialog::on_returnButton_clicked() {
    if (!user) return;
    int bookID = getSelection();
    lib.returnBook(user->elem.identifier, bookID);
    displayTable();
}

// 获取当前选中的书籍ID
int UserInfoDialog::getSelection() {
    int curRow = ui->tableView->currentIndex().row();
    QAbstractItemModel* itemModel = ui->tableView->model();
    QModelIndex itemIdx = itemModel->index(curRow, 1);
    QVariant dataTmp = itemModel->data(itemIdx);
    return dataTmp.toInt();
}

// 重载函数，根据索引获取选中的书籍ID
int UserInfoDialog::getSelection(const QModelIndex &index) {
    int curRow = index.row();
    QAbstractItemModel* itemModel = ui->tableView->model();
    QModelIndex itemIdx = itemModel->index(curRow, 1);
    QVariant dataTmp = itemModel->data(itemIdx);
    return dataTmp.toInt();
}

// 更新用户信息函数，将用户输入的内容保存到user对象中
void UserInfoDialog::updateUserInfo() {
    QString name = ui->nameEdit->text();
    int id = ui->idEdit->text().toInt();
    bool type = ui->adminBox->isChecked();
    if (user) {
        user->elem.name = name.toStdString();
        user->elem.identifier = id;
        user->elem.type = type;
    } else {
        user = lib.add(UserInfo(name.toStdString(), id, type));
    }
}

// 确认按钮点击事件处理
void UserInfoDialog::on_buttonBox_accepted() {
    if (checkUserInfo()) {
        updateUserInfo();
        accept();
    }
}

// 删除按钮点击事件处理
void UserInfoDialog::on_deleteButton_clicked() {
    if (!user) return;
    if (QMessageBox::warning(this, tr("确认删除"), tr("要永久删除此用户吗？此操作不可撤销。"),
                             QMessageBox::Ok | QMessageBox::Cancel)
            == QMessageBox::Cancel) {
        return;
    }
    if (!lib.del(user)) {
        if (QMessageBox::warning(this, tr("警告"),
                             tr("该用户还有 ") + QString::number(user->elem.books.size())
                             + tr(" 本书未归还。要强制删除吗？"),
                             QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Cancel) {
            return;
        }
        lib.del(user, true);
    }
    accept();
}

// 表格点击事件处理，启用归还按钮
void UserInfoDialog::on_tableView_clicked() {
    if (user) {
        ui->returnButton->setDisabled(false);
    }
}

// 检查用户信息有效性
bool UserInfoDialog::checkUserInfo() {
    int id = ui->idEdit->text().toInt();
    auto findRes = lib.findUser(id);
    if (findRes && findRes != user) {
        QMessageBox::warning(this, tr("重复的编号"), tr("用户编号不能重复。"), QMessageBox::Ok);
        return false;
    }
    return id >= 0;
}

// 借书按钮点击事件处理，打开选择对话框
void UserInfoDialog::on_borrowButton_clicked() {
    SelectDialog selectDlg(this, -1, user->elem.identifier);
    connect(&selectDlg, SIGNAL(sendData(QString)), this, SLOT(receiveBookData(QString)));
    selectDlg.exec();
}

// 密码修改按钮点击事件处理，打开密码修改对话框
void UserInfoDialog::on_pwdButton_clicked() {
    PasswordDialog pwdDlg(this, user);
    connect(&pwdDlg, SIGNAL(sendPwdData(QString)), this, SLOT(receivePwdData(QString)));
    pwdDlg.exec();
}

// 表格双击事件处理，打开图书信息对话框
void UserInfoDialog::on_tableView_doubleClicked(const QModelIndex &index) {
    int bookID = getSelection(index);
    BookInfoDialog bookDialog(this, bookID);
    bookDialog.exec();
    displayTable();
}
