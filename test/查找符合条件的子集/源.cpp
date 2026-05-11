#include <iostream>
#include <vector>
#include <string>
#include <cctype>

//比较函数：不区分大小写
bool comparechar_nocase(char c1, char c2) {
    return std::tolower(c1) == std::tolower(c2);
}

//函数对象类
template<typename T>
class myCompare {
public:
    bool operator()(const T& t1, const T& t2) const {
        return t1 == t2;
    }
};

//查找算法实现
template<typename ForwardIterator1, typename ForwardIterator2, class BinaryPredicate>
ForwardIterator1 find_first_location(ForwardIterator1 first1, ForwardIterator1 last1,
                                     ForwardIterator2 first2, ForwardIterator2 last2,
                                     BinaryPredicate pred) {
    //如果子序列为空，直接返回起始位置
    if (first2 == last2) return first1;

    ForwardIterator1 it1 = first1;
    while (it1 != last1) {
        //尝试匹配
        ForwardIterator1 m_it1 = it1;
        ForwardIterator2 m_it2 = first2;
        
        //只要没到结尾且元素相等，就继续比对
        while (m_it1 != last1 && m_it2 != last2 && pred(*m_it1, *m_it2)) {
            ++m_it1;
            ++m_it2;
        }
        
        //如果子序列遍历完了，说明找到了
        if (m_it2 == last2) {
            return it1;
        }
        
        //否则继续下一个位置
        ++it1;
    }
    return last1;
}

int main(){
    std::string szsrc = "Ann and Andy's anniversary is in April";
    std::string szneedle = "And";
    auto it = find_first_location(szsrc.begin(), szsrc.end(),
                                  szneedle.begin(), szneedle.end(),
                                  comparechar_nocase );
    if(it != szsrc.end()) std::cout << *it << std::endl; //输出 'a'

    std::vector<int> ver = {2,5,4,1,5,4,2,5};
    int ar[] = {5,4,1};
    auto it1 = ver.begin();
    auto it2 = ver.end();
    auto itt = find_first_location(it1, it2, ar, ar+3, myCompare<int>() );
    
    if(itt != ver.end()) {
        std::cout << *itt << " 为第 N 个元素 :" << std::distance(it1, itt) << std::endl;
    }
    return 0;
}