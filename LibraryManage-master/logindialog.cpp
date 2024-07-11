#include "logindialog.h"
#include "ui_logindialog.h"
#include <fstream> // 包含文件操作所需的头文件

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 如果已经保存过登录信息，则勾选复选框并读取保存的数据
    if (isSaved()) {
        ui->checkBox->setChecked(true);
        readSavedData();
    }
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::isSaved()
{
    // 检查是否存在保存的登录信息文件
    std::ifstream input("saved");
    return input.good(); // 如果文件存在且可以读取，则返回 true
}

void LoginDialog::readSavedData()
{
    // 如果复选框未选中，则不读取保存的数据
    if (!ui->checkBox->isChecked()) return;

    // 读取保存的用户名和密码
    std::ifstream input("saved");
    if (input.good()) {
        std::getline(input, username); // 读取用户名
        std::getline(input, password); // 读取密码
        ui->unameEdit->setText(QString::fromStdString(username));
        ui->pwdEdit->setText(QString::fromStdString(password));
    }
}

void LoginDialog::on_buttonBox_accepted()
{
    // 获取用户名和密码输入框中的内容
    username = ui->unameEdit->text().toStdString();
    password = ui->pwdEdit->text().toStdString();

    // 调用库函数进行登录验证
    if (lib.login(username, password)) {
        // 如果登录成功，根据复选框状态保存登录信息到文件
        if (ui->checkBox->isChecked()) {
            std::ofstream output("saved");
            output << username << std::endl << password << std::endl;
        }

        // 设置全局变量记录登录用户ID并接受对话框
        loginUserID = lib.findUser(username)->elem.identifier;
        accept();
    } else {
        // 如果登录失败，弹出警告对话框
        QMessageBox::warning(this, tr("提示"), tr("用户名或密码错误。"), QMessageBox::Ok);
    }
}
