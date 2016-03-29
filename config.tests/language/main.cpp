/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtSerialBus module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtCore/qglobal.h>

#include <memory>
#include <type_traits>

struct Base
{
    virtual void overrideMe() = 0;
};

struct Test : public Base
{
    // strongly typed enums
    enum struct Enum : char {
        EnumA, EnumB
    };

    Test() = default;   // defaulted and
    Test(const Test&) = delete; // deleted ctors

    explicit Test(int i)
        : Test(0, i) { }
    Test(int a, int i)
        : m_intA(a), m_intI(i) {}

    // std::is_same
    template <typename T, typename ... Ts> struct IsType { enum { value = false }; };
    template <typename T, typename T1, typename ... Ts> struct IsType<T, T1, Ts...> {
        enum { value = std::is_same<T, T1>::value || IsType<T, Ts...>::value };
    };

    // std::is_pod
    template <typename T> void test(T) const {
        static_assert(std::is_pod<T>::value, "Only POD supported.");
        static_assert(IsType<T, int>::value, "Only int supported.");
    }

    // variadics
    template <typename ... Args>
    void setValues(Args ... values)
    {
        const int size = sizeof...(Args);
        int tmp[size] = { (test(values), void(), '0')... };
        (void) (tmp);
    }

    // override keyword
    void overrideMe() override {}

    // const and constexpr
    Q_DECL_CONSTEXPR qint64 first() const Q_DECL_NOTHROW { return m_intA; }
    Q_DECL_RELAXED_CONSTEXPR void setFirst(int a) Q_DECL_NOTHROW { m_intA = a; }

    // non-static data member initializers
    int m_intA = 0;
    int m_intI = 0;
};

using func = int (*) (int, int);
int total(int a, int i) { return a + i; }

int main(int /*argc*/, char** /*argv*/)
{
    // nullptr
    Test *t = nullptr;
    Test stackT(155);
    t = &stackT;

    // variadics
    t->setValues(1, 2, 3, 4, 5);

    // lambda and auto
    auto m = [](int a, int b) -> int {
        return a * b;
    };

    // decltype
    decltype(m) multiply = m;
    multiply(t->m_intA, t->m_intI);

    // alias templates
    func f = total;
    f(t->m_intA, t->m_intI);

    return 0;
}
