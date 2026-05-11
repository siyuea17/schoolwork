#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>

using namespace std;

// 前置声明
class Strategy;

// ============== 1. 商品类 goods ==============
class Goods {
private:
    string shopName;    // 店铺名称
    string goodsName;   // 商品名称
    double price;       // 价格

public:
    Goods(const string& shopName, const string& goodsName, double price)
        : shopName(shopName), goodsName(goodsName), price(price) {
    }

    // Getters
    string getShopName() const { return shopName; }
    string getGoodsName() const { return goodsName; }
    double getPrice() const { return price; }

    // 显示商品信息
    void display() const {
        cout << "商品: " << goodsName << ", 店铺: " << shopName
            << ", 价格: " << price << "元" << endl;
    }
};

// ============== 2. 优惠活动策略类 strategy (抽象类) ==============
class Strategy {
public:
    virtual ~Strategy() = default;
    // 纯虚函数：计算优惠后应付金额
    virtual double calFee(double totalAmount) const = 0;

    // 获取策略描述
    virtual string getDescription() const = 0;
};

// ============== 3. 满减优惠类 full_dis_strategy ==============
class FullDisStrategy : public Strategy {
private:
    double fullAmount;    // 满多少金额
    double discount;      // 减少的金额

public:
    FullDisStrategy(double fullAmount, double discount)
        : fullAmount(fullAmount), discount(discount) {
        if (fullAmount <= 0 || discount <= 0) {
            throw invalid_argument("满减金额和减少金额必须大于0");
        }
    }

    double calFee(double totalAmount) const override {
        if (totalAmount < fullAmount) {
            return totalAmount; // 未达到满减条件
        }

        // 计算可以享受满减的次数
        int times = static_cast<int>(totalAmount / fullAmount);
        double reduction = times * discount;

        // 确保减少金额不超过总金额
        return max(0.0, totalAmount - reduction);
    }

    string getDescription() const override {
        return "满" + to_string(fullAmount) + "减" + to_string(discount);
    }
};

// ============== 4. 折扣优惠类 discount_strategy ==============
class DiscountStrategy : public Strategy {
private:
    double discountRate;  // 折扣率 (0.0 - 1.0)

public:
    DiscountStrategy(double discountRate)
        : discountRate(discountRate) {
        if (discountRate <= 0 || discountRate > 1.0) {
            throw invalid_argument("折扣率必须在(0, 1]范围内");
        }
    }

    double calFee(double totalAmount) const override {
        return totalAmount * discountRate;
    }

    string getDescription() const override {
        return to_string(static_cast<int>(discountRate * 100)) + "折";
    }
};

// ============== 5. 商店购物列表类 Store_Shopping_list ==============
class StoreShoppingList {
private:
    string shopName;                          // 店铺名称
    vector<shared_ptr<Goods>> goodsList;      // 该店铺购买的商品列表
    shared_ptr<Strategy> strategy;            // 优惠策略

public:
    StoreShoppingList(const string& shopName)
        : shopName(shopName), strategy(nullptr) {
    }

    // 设置优惠策略
    void setStrategy(shared_ptr<Strategy> newStrategy) {
        strategy = newStrategy;
    }

    // 添加商品到购物列表
    void addGoods(shared_ptr<Goods> goods) {
        if (goods->getShopName() != shopName) {
            throw invalid_argument("商品店铺与购物列表店铺不匹配");
        }
        goodsList.push_back(goods);
    }

    // 获取店铺名称
    string getShopName() const { return shopName; }

    // 计算原始总金额
    double calculateOriginalTotal() const {
        double total = 0.0;
        for (const auto& goods : goodsList) {
            total += goods->getPrice();
        }
        return total;
    }

    // 计算优惠后应付金额
    double calculateFinalFee() const {
        double originalTotal = calculateOriginalTotal();
        if (strategy) {
            return strategy->calFee(originalTotal);
        }
        return originalTotal; // 无优惠策略
    }

    // 获取节省金额
    double getSavedAmount() const {
        double originalTotal = calculateOriginalTotal();
        double finalFee = calculateFinalFee();
        return originalTotal - finalFee;
    }

    // 显示购物列表信息
    void display() const {
        cout << "===== " << shopName << " 购物列表 =====" << endl;
        for (const auto& goods : goodsList) {
            goods->display();
        }

        double originalTotal = calculateOriginalTotal();
        double finalFee = calculateFinalFee();
        double saved = getSavedAmount();

        cout << "原始总金额: " << originalTotal << "元" << endl;
        if (strategy) {
            cout << "优惠策略: " << strategy->getDescription() << endl;
            cout << "优惠后金额: " << finalFee << "元" << endl;
            cout << "节省金额: " << saved << "元" << endl;
        }
        else {
            cout << "无优惠活动" << endl;
        }
        cout << "=================================" << endl;
    }
};

// ============== 6. 购物车类 shopping_car ==============
class ShoppingCar {
private:
    vector<shared_ptr<StoreShoppingList>> storeLists;  // 各店铺购物列表

public:
    // 添加商品到购物车
    void addGoodsToCar(shared_ptr<Goods> goods) {
        // 查找是否已有该店铺的购物列表
        auto it = find_if(storeLists.begin(), storeLists.end(),
            [&goods](const shared_ptr<StoreShoppingList>& list) {
                return list->getShopName() == goods->getShopName();
            });

        if (it == storeLists.end()) {
            // 创建新的店铺购物列表
            auto newList = make_shared<StoreShoppingList>(goods->getShopName());
            newList->addGoods(goods);
            storeLists.push_back(newList);
        }
        else {
            // 添加到现有购物列表
            (*it)->addGoods(goods);
        }
    }

    // 为特定店铺设置优惠策略
    void setStrategyForShop(const string& shopName, shared_ptr<Strategy> strategy) {
        auto it = find_if(storeLists.begin(), storeLists.end(),
            [&shopName](const shared_ptr<StoreShoppingList>& list) {
                return list->getShopName() == shopName;
            });

        if (it != storeLists.end()) {
            (*it)->setStrategy(strategy);
        }
        else {
            throw runtime_error("未找到店铺: " + shopName);
        }
    }

    // 计算购物车总原始金额
    double calculateOriginalTotal() const {
        double total = 0.0;
        for (const auto& list : storeLists) {
            total += list->calculateOriginalTotal();
        }
        return total;
    }

    // 计算购物车优惠后总金额
    double calculateFinalTotal() const {
        double total = 0.0;
        for (const auto& list : storeLists) {
            total += list->calculateFinalFee();
        }
        return total;
    }

    // 计算总节省金额
    double calculateTotalSaved() const {
        return calculateOriginalTotal() - calculateFinalTotal();
    }

    // 显示购物车详情
    void displayCart() const {
        cout << "============== 购物车详情 ==============" << endl;
        for (const auto& list : storeLists) {
            list->display();
        }

        double originalTotal = calculateOriginalTotal();
        double finalTotal = calculateFinalTotal();
        double totalSaved = calculateTotalSaved();

        cout << "===== 购物车总计 =====" << endl;
        cout << "商品原始总金额: " << originalTotal << "元" << endl;
        cout << "优惠后总金额: " << finalTotal << "元" << endl;
        cout << "总共节省: " << totalSaved << "元" << endl;
        if (totalSaved > 0) {
            cout << "节省比例: " << (totalSaved / originalTotal * 100) << "%" << endl;
        }
        cout << "======================================" << endl;
    }
};

// ============== 7. 商品管理类 goodsManage ==============
class GoodsManage {
private:
    vector<shared_ptr<Goods>> allGoods;  // 所有商品

public:
    // 添加商品
    void addGoods(shared_ptr<Goods> goods) {
        allGoods.push_back(goods);
    }

    // 按店铺名称获取商品列表
    vector<shared_ptr<Goods>> getGoodsByShop(const string& shopName) const {
        vector<shared_ptr<Goods>> result;
        for (const auto& goods : allGoods) {
            if (goods->getShopName() == shopName) {
                result.push_back(goods);
            }
        }
        return result;
    }

    // 按商品名称搜索
    vector<shared_ptr<Goods>> searchGoodsByName(const string& goodsName) const {
        vector<shared_ptr<Goods>> result;
        for (const auto& goods : allGoods) {
            if (goods->getGoodsName().find(goodsName) != string::npos) {
                result.push_back(goods);
            }
        }
        return result;
    }

    // 显示所有商品
    void displayAllGoods() const {
        cout << "============== 所有商品 ==============" << endl;
        for (const auto& goods : allGoods) {
            goods->display();
        }
        cout << "=====================================" << endl;
    }
};