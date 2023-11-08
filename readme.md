# mymuduo
## nocopyable

<img src=".\img\AIDL Framwork.png" alt="&quot;D:\github\mymuduo\img\AIDL Framwork.png&quot;" style="zoom:150%;" />

当派生类定义了拷贝或移动构造函数，必须显式地调用基类的拷贝或移动构造函数，否则会调用基类的默认构造函数

```c++
class Base {
public:
    Base() { std::cout << "Base default constructor" << std::endl; }
    Base(const Base &b) { std::cout << "Base copy constructor" << std::endl; }
    Base(Base &&b) { std::cout << "Base move constructor" << std::endl; }
};

class Derived : public Base {
public:
    Derived() { std::cout << "Derived default constructor" << std::endl; }
    Derived(const Derived &d) { std::cout << "Derived copy constructor" << std::endl; }
    Derived(Derived &&d) { std::cout << "Derived move constructor" << std::endl; }
};

int main() {
    Derived d;
    std::cout << std::endl;
    Derived d0(d);
    std::cout << std::endl;
    Derived d1 = d;
    std::cout << std::endl;
    Derived d2{d};
    return 0;
}
输出：
Base default constructor
Derived default constructor

Base default constructor
Derived copy constructor

Base default constructor
Derived copy constructor

Base default constructor
Derived copy constructor
```

需要在子类中显示调用：

```c++
class Derived : public Base {
public:
    Derived() { std::cout << "Derived default constructor" << std::endl; }
    Derived(const Derived &d)
        : Base(d) { std::cout << "Derived copy constructor" << std::endl; }
    Derived(Derived &&d)
        : Base(std::move(d)) { std::cout << "Derived move constructor" << std::endl; }
};
输出：
Base default constructor
Derived default constructor

Base copy constructor
Derived copy constructor

Base copy constructor
Derived copy constructor

Base copy constructor
Derived copy constructor
```

当派生类未定义拷贝或移动构造函数，若基类有拷贝和移动构造函数，则派生类将获得合成的拷贝和移动构造函数，且调用的是基类的拷贝或移动构造函数

```c++
class Base {
public:
    Base() { std::cout << "Base default constructor" << std::endl; }
    Base(const Base &b) { std::cout << "Base copy constructor" << std::endl; }
    Base(Base &&b) { std::cout << "Base move constructor" << std::endl; }
};
class Derived : public Base { }
};
int main() {
    Derived d;
    Derived d1(d);
    Derived d2(std::move(d));
    return 0;
}
输出：
Base default constructor
Base copy constructor
Base move constructor
```



