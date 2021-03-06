#include <cassert>
#include <memory>
#include <functional>
#include <initializer_list>
#include <iostream> // debugging

template<class T>
class List
{
    struct Item
    {
        Item(T v, std::shared_ptr<const Item> const & tail) : _val(v), _next(tail) {}
        // For debugging only
        ~Item() { std::cout << "! " << _val << std::endl; }
        T _val;
        std::shared_ptr<const Item> _next;
    };
    friend Item;
    explicit List(std::shared_ptr<const Item> const & items) : _head(items) {}
public:
    // Empty list
    List() {}
    // Cons
    List(T v, List const & tail) : _head(std::make_shared<Item>(v, tail._head)) {}
    // From initializer list
    List(std::initializer_list<T> init)
    {
        for (auto it = std::rbegin(init); it != std::rend(init); ++it)
        {
            _head = std::make_shared<Item>(*it, _head);
        }
    }

    bool isEmpty() const { return !_head; } // conversion to bool
    T front() const
    {
        assert(!isEmpty());
        return _head->_val;
    }
    List popped_front() const
    {
        assert(!isEmpty());
        return List(_head->_next);
    }
    // Additional utilities
    List prepended(T v) const
    {
        return List(v, *this);
    }
    List insertedAt(int i, T v) const
    {
        if (i == 0)
            return prepended(v);
        else {
            assert(!isEmpty());
            return List<T>(front(), popped_front().insertedAt(i - 1, v));
        }
    }
    // For debugging
    int headCount() const { return _head.use_count(); }
private:
    std::shared_ptr<const Item> _head;
};


template<class T>
List<T> concat(List<T> const & a, List<T> const & b)
{
    if (a.isEmpty())
        return b;
    return List<T>(a.front(), concat(a.popped_front(), b));
}

template<class U, class T, class F>
List<U> fmap(F f, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(T)>>::value, 
                 "fmap requires a function type U(T)");
    if (lst.isEmpty()) 
        return List<U>();
    else
        return List<U>(f(lst.front()), fmap<U>(f, lst.popped_front()));
}

template<class T, class P>
List<T> filter(P p, List<T> lst)
{
    static_assert(std::is_convertible<P, std::function<bool(T)>>::value, 
                 "filter requires a function type bool(T)");
    if (lst.isEmpty())
        return List<T>();
    if (p(lst.front()))
        return List<T>(lst.front(), filter(p, lst.popped_front()));
    else
        return filter(p, lst.popped_front());
}

template<class T, class U, class F>
U foldr(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(T, U)>>::value, 
                 "foldr requires a function type U(T, U)");
    if (lst.isEmpty())
        return acc;
    else
        return f(lst.front(), foldr(f, acc, lst.popped_front()));
}

template<class T, class U, class F>
U foldl(F f, U acc, List<T> lst)
{
    static_assert(std::is_convertible<F, std::function<U(U, T)>>::value, 
                 "foldl requires a function type U(U, T)");
    if (lst.isEmpty())
        return acc;
    else
        return foldl(f, f(acc, lst.front()), lst.popped_front());
}

template<class T, class F>
void forEach(List<T> lst, F f) 
{
    static_assert(std::is_convertible<F, std::function<void(T)>>::value, 
                 "forEach requires a function type void(T)");
    if (!lst.isEmpty()) {
        f(lst.front());
        forEach(lst.popped_front(), f);
    }
}

template<class T>
List<T> reversed(List<T> const & lst)
{
    return foldl([](List<T> const & acc, T v)
    {
        return List<T>(v, acc);
    }, List<T>(), lst);
}


template<class Beg, class End>
auto fromIt(Beg it, End end) -> List<typename Beg::value_type>
{
    typedef typename Beg::value_type T;
    if (it == end)
        return List<T>();
    T item = *it;
    return List<T>(item, fromIt(++it, end));
}

// Pass lst by value not reference!
template<class T>
void print(List<T> lst)
{
    if (lst.isEmpty()) {
        std::cout << std::endl;
    }
    else {
        std::cout << "(" << lst.front() << ", " << lst.headCount() - 1 << ") ";
        print(lst.popped_front());
    }
}
