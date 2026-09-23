#include <iostream>
#include <string>
#include "C:\\Users\\MR\\source\\repos\\AddressBookDLL\\AddressBookDLL\\AddressBook.h"
#pragma comment(lib, "C:\\Users\\MR\\source\\repos\\AddressBookDLL\\x64\\Debug\\AddressBook.lib")

int main() {
    // 创建一个最大容量 100 的通讯录
    AddressBook ab(100);
    
    ab.loadFromFile("C:\\Users\\MR\\source\\repos\\AddressBookDLL\\AddressBookDLL\\AddressBook.txt");

    ab.displayAll();

    // 按姓名查找
    std::string searchName = "张三";
    int idx = ab.findByName(searchName);

    // 修改电话
    if (ab.modifyTel("张三", "13900001111"))
        std::cout << "修改成功" << std::endl;
    else
        std::cout << "修改失败，用户不存在" << std::endl;

    // 保存到文件
    ab.saveToFile("test_save.txt");
    std::cout << "\n已保存到 test_save.txt" << std::endl;


    AddressBook ab2(100);
    ab2.loadFromFile("test_save.txt");
    std::cout << "\n=== 从文件加载后的通讯录 ===" << std::endl;
    ab2.displayAll();

    std::cout << "\n=== 分开显示朋友和一般条目 ===" << std::endl;
    ab2.showSeparate();

    return 0;
}