#include <iostream>
#include <string>
#include <iomanip>

using namespace std;

class ElectronicDevice {
private:
    // 普通成员变量
    int devId;          // 设备编号
    string devName;     // 设备名称
    string devType;     // 设备类型
    double price;       // 购买价格
    // 静态成员变量（所有对象共享）
    static int totalDeviceNum;   // 设备总数
    static double totalPrice;    // 总购买金额
public:
    // 无参构造函数
    // 带参构造函数
    ElectronicDevice() {
        devId = 0;
        devName = "未知设备";
        devType = "未知类型";
        price = 0.0;
        totalDeviceNum++;
    }
    ElectronicDevice(int id, string name, string type, double p) {
        devId = id;
        devName = name;
        devType = type;
        price = p;
        totalDeviceNum++;
        totalPrice += p;
    }
    // 修改设备信息
    void setDeviceInfo(int id, string name, string type, double p) {
        // 先扣除原价格，再累加新价格
        totalPrice = totalPrice - price + p;
        devId = id;
        devName = name;
        devType = type;
        price = p;
    }
    // 输出单个设备信息
    void getDeviceInfo() {
        cout << fixed << setprecision(2);
        cout << "设备编号：" << devId << "，名称：" << devName
            << "，类型：" << devType << "，购买价格：" << price << "元" << endl;
    }

    // 获取价格等级
    string getPriceLevel() {
        if (price >= 5000) {
            return "高端";
        }
        else if (price >= 2000) {
            return "中端";
        }
        else {
            return "入门";
        }
    }
    // 静态成员函数：输出总统计信息
    static void getTotalInfo() {
        cout << fixed << setprecision(2);
        cout << "设备总数：" << totalDeviceNum << "，总购买金额：" << totalPrice << "元" << endl;
    }
};
int ElectronicDevice::totalDeviceNum = 0;
double ElectronicDevice::totalPrice = 0;

int main() {
    // 创建无参构造对象
    ElectronicDevice dev1;
    // 修改dev1信息
    dev1.setDeviceInfo(1001, "小米14", "手机", 4999.0);

    // 创建带参构造对象
    ElectronicDevice dev2(1002, "MacBook Pro", "笔记本", 12999.0);
    ElectronicDevice dev3(1003, "iPad 10", "平板", 1999.0);

    // 输出单个设备信息和价格等级
    cout << "设备1信息：";
    dev1.getDeviceInfo();
    cout << "设备1价格等级：" << dev1.getPriceLevel() << endl;

    cout << "设备2信息：";
    dev2.getDeviceInfo();
    cout << "设备2价格等级：" << dev2.getPriceLevel() << endl;

    cout << "设备3信息：";
    dev3.getDeviceInfo();
    cout << "设备3价格等级：" << dev3.getPriceLevel() << endl;

    // 输出总统计信息（静态函数调用：类名::函数名 或 对象.函数名）
    cout << "\n所有设备统计信息：";
    ElectronicDevice::getTotalInfo();

    return 0;
}