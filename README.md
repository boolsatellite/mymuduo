## muduo库核心代码实现

### nocopyable

muduo库中为了阻止某些类的拷贝构造和赋值采用了私有继承nocopyable类，nocopyable将拷贝构造和赋值运算符delete掉了，这样有效的阻止了派生类的复制，优于将拷贝构造delete或设为私有方法，这种方法可以清晰的看出一个类是否允许被复制



