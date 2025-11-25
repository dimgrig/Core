#ifndef VISITOR_H
#define VISITOR_H

#include <iostream>
#include <memory>

// ---------------------------------
// Generic visitor base
template <typename ... Types>
class Visitor;

template <typename T>
class Visitor<T> {
public:
    virtual void visit(T* t) = 0;
};

template <typename T, typename ... Types>
class Visitor<T, Types ...> : public Visitor<Types ...> {
public:
    using Visitor<Types ...>::visit;
    virtual void visit(T* t) = 0;
};



template <typename ... Types>
class SharedPtrVisitor;

template <typename T>
class SharedPtrVisitor<T> {
public:
    virtual void visit(std::shared_ptr<T> t) = 0;
};

template <typename T, typename ... Types>
class SharedPtrVisitor<T, Types ...> : public SharedPtrVisitor<Types ...> {
public:
    using SharedPtrVisitor<Types ...>::visit;
    virtual void visit(std::shared_ptr<T> t) = 0;
};

// ----------------------------------------------
//
template <typename Base, typename ... >
class LambdaVisitor;

template <typename Base, typename T1, typename F1>
class LambdaVisitor<Base, Visitor<T1>, F1> : private F1, public Base {
public:
    LambdaVisitor(F1&& f1) : F1(std::move(f1)) {}
    LambdaVisitor(const F1& f1) : F1(f1) {}
    void visit(T1* t) override { return F1::operator()(t); }
};

template <typename Base, typename T1, typename ... T, typename F1, typename ... F>
class LambdaVisitor<Base, Visitor<T1, T ...>, F1, F ...> : private F1, public LambdaVisitor<Base, Visitor<T ...>, F ...> {
public:
    LambdaVisitor(F1&& f1, F&& ... f) : F1(std::move(f1)), LambdaVisitor<Base, Visitor<T ...>, F ...>(std::forward<F>(f) ...) {}
    LambdaVisitor(const F1& f1, F&& ... f) : F1(f1), LambdaVisitor<Base, Visitor<T ...>, F ...>(std::forward<F>(f) ...) {}
    void visit(T1* t) override { return F1::operator()(t); }
};

template <typename Base, typename ... F>
auto lambda_visitor(F&& ... f) {
    return LambdaVisitor<Base, Base, F ...>(std::forward<F>(f) ...);
}
//
// ----------------------------------------------

#endif // VISITOR_H
