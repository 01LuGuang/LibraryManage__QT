#include "passworddialog.h"
#include "ui_passworddialog.h"
#include "userinfodialog.h"

PasswordDialog::PasswordDialog(QWidget *parent, Node<UserInfo> *_user) :
    QDialog(parent),
    ui(new Ui::PasswordDialog),
    user(_user)
{
    ui->setupUi(this);

    // 如果是管理员登录，则隐藏旧密码输入框和标签
    if (isLoginAdmin) {
        ui->oldPwdEdit->setHidden(true);
        ui->oldPwdLabel->setHidden(true);
    }
}

PasswordDialog::~PasswordDialog()
{
    delete ui;
}

bool PasswordDialog::checkPassword()
{
    // 检查输入的旧密码是否正确
    string  realOldPwd  = user->elem.password;
    QString inputOldPwd = ui->oldPwdEdit->text();
    return realOldPwd == inputOldPwd.toStdString();
}

void PasswordDialog::on_buttonBox_accepted()
{
    if (!isLoginAdmin) {
        // 非管理员需要验证旧密码
        if (checkPassword()) {
            emit sendPwdData(ui->newPwdEdit->text());
        } else {
            // 提示输入正确的旧密码或联系管理员
            QMessageBox::information(this, tr("信息"), tr("请输入正确的旧密码。<br>若忘记请联系管理员修改密码。"), QMessageBox::Ok);
            return;
        }
    } else {
        // 管理员可以直接修改密码
        emit sendPwdData(ui->newPwdEdit->text());
    }

    // 接受对话框
    accept();
}
