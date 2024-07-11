#include "librarymain.h"
#include "logindialog.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>

// 全局变量声明
Library lib;
int loginUserID;
bool isLoginAdmin;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序的字体
    a.setFont(QFont("微软雅黑", 9));

    // 语言文件读取和安装
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LibraryManage_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // 读取图书数据
    if (lib.read("user.csv", "book.csv")) {
        loginUserID = -1; // 初始用户ID设为-1，表示未登录
        isLoginAdmin = true; // 设置登录状态为管理员
        LibraryMain w;
        w.show();
        return a.exec();
    }

    // 显示登录对话框
    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        LibraryMain w;
        w.show();
        return a.exec();
    }

    return 0;
}
