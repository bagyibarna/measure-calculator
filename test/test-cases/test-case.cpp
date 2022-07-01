#include <doctest/doctest.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "measure-calculator/measure-calculator.hpp"
#include "measure-calculator/defaults.hpp"

using namespace Calc;

struct Asserter {
    Asserter(SpecBuilder builder) {
        auto built = std::move(builder).Build();
        CHECK_UNARY(std::holds_alternative<Spec>(built));
        spec = std::move(std::get<Spec>(built));
    }

    Spec spec;

    static auto WrapForCheck(Error e) { return e; }
    static auto WrapForCheck(double d) { return doctest::Approx(d); }

    template <class T>
    void operator()(std::string_view str, T expected,
                    std::optional<T> no_whitespace_expected = std::nullopt) const {
        using PureT = std::remove_cvref_t<T>;
        auto result = Evaluate(spec, str);
        CHECK_UNARY(std::holds_alternative<PureT>(result));
        CHECK_EQ(std::get<PureT>(result), WrapForCheck(expected));

        std::string no_whitespace(str.begin(), str.end());
        no_whitespace.erase(std::remove_if(no_whitespace.begin(), no_whitespace.end(), ::isspace),
                            no_whitespace.end());

        auto no_whitespace_result = Evaluate(spec, no_whitespace);
        CHECK_UNARY(std::holds_alternative<PureT>(no_whitespace_result));
        CHECK_EQ(std::get<PureT>(no_whitespace_result),
                 WrapForCheck(no_whitespace_expected.value_or(expected)));
    }
};

template<class T>
auto operator+(T left, T right) {
    left.insert(left.end(), std::move_iterator(right.begin()), std::move_iterator(right.end()));
    return left;
}

const SpecBuilder kDefaultBuilder{
    .unary_ops = Defaults::kNegateUnaryOp,
    .binary_ops = Defaults::kArithmeticBinaryOps,
    .unary_funs = Defaults::kBasicUnaryFuns + Defaults::kExponentialUnaryFuns +
                  Defaults::kTrigonometricUnaryFuns,
    .binary_funs = Defaults::kBasicBinaryFuns,
    .constants = Defaults::kBasicConstants,
    .measures = {Defaults::kLinearMeasure},
};

TEST_CASE("Numbers") {
    Asserter assertion({});

    SUBCASE("Integer-like") {
        assertion("0", 0.);
        assertion("1", 1.);
        assertion(" 1", 1.);
        assertion("1 ", 1.);
        assertion(" 1 ", 1.);
        assertion("01", 1.);
        assertion("10", 10.);
        assertion("123456789", 123456789.);
    }

    SUBCASE("Decimal Float") {
        assertion("0.0", 0.);
        assertion("0.1", 0.1);
        assertion("1.0", 1.0);
        assertion("01.01", 1.01);
        assertion("10.10", 10.1);
        assertion("123456789.123456789", 123456789.123456789);
    }

    SUBCASE("Exponential without Decimals") {
        assertion("0e1", 0.);
        assertion("0e-1", 0.);
        assertion("0e5", 0.);
        assertion("0e-5", 0.);
        assertion("0e1000", 0.);
        assertion("0e-1000", 0.);

        assertion("1e1", 1e1);
        assertion("1e-1", 1e-1);
        assertion("1e5", 1e5);
        assertion("1e-5", 1e-5);
        assertion("1e10", 1e10);
        assertion("1e-10", 1e-10);

        assertion("123456789e1", 123456789e1);
        assertion("123456789e-1", 123456789e-1);
        assertion("123456789e10", 123456789e10);
        assertion("123456789e-10", 123456789e-10);

        assertion("0E1", 0.);
        assertion("0E-1", 0.);
        assertion("0E5", 0.);
        assertion("0E-5", 0.);
        assertion("0E1000", 0.);
        assertion("0E-1000", 0.);

        assertion("1E1", 1E1);
        assertion("1E-1", 1E-1);
        assertion("1E5", 1E5);
        assertion("1E-5", 1E-5);
        assertion("1E10", 1E10);
        assertion("1E-10", 1E-10);

        assertion("123456789E1", 123456789E1);
        assertion("123456789E-1", 123456789E-1);
        assertion("123456789E10", 123456789E10);
        assertion("123456789E-10", 123456789E-10);
    }

    SUBCASE("Exponential Float") {
        assertion("0.0e0", 0e0);
        assertion("0.1e0", 0.1e0);
        assertion("1.0e0", 1.0e0);
        assertion("01.01e0", 1.01e0);
        assertion("10.10e0", 10.1e0);
        assertion("123456789.123456789e0", 123456789.123456789);

        assertion("0.0e1", 0.0e1);
        assertion("0.1e1", 0.1e1);
        assertion("1.0e1", 1.0e1);
        assertion("01.01e1", 01.01e1);
        assertion("10.10e1", 10.10e1);
        assertion("123456789.123456789e1", 123456789.123456789e1);

        assertion("0.0e-1", 0.0e-1);
        assertion("0.1e-1", 0.1e-1);
        assertion("1.0e-1", 1.0e-1);
        assertion("01.01e-1", 01.01e-1);
        assertion("10.10e-1", 10.10e-1);
        assertion("123456789.123456789e-1", 123456789.123456789e-1);

        assertion("0.0e10", 0.0e10);
        assertion("0.1e10", 0.1e10);
        assertion("1.0e10", 1.0e10);
        assertion("01.01e10", 01.01e10);
        assertion("10.10e10", 10.10e10);
        assertion("123456789.1234567891e10", 123456789.123456789e10);

        assertion("0.0e-10", 0.0e-10);
        assertion("0.1e-10", 0.1e-10);
        assertion("1.0e-10", 1.0e-10);
        assertion("01.01e-10", 01.01e-10);
        assertion("10.10e-10", 10.10e-10);
        assertion("123456789.123456789e-10", 123456789.123456789e-10);

        assertion("0.0E0", 0E0);
        assertion("0.1E0", 0.1E0);
        assertion("1.0E0", 1.0E0);
        assertion("01.01E0", 1.01E0);
        assertion("10.10E0", 10.1E0);
        assertion("123456789.123456789E0", 123456789.123456789E0);

        assertion("0.0E1", 0.0E1);
        assertion("0.1E1", 0.1E1);
        assertion("1.0E1", 1.0E1);
        assertion("01.01E1", 01.01E1);
        assertion("10.10E1", 10.10E1);
        assertion("123456789.123456789E1", 123456789.123456789E1);

        assertion("0.0E-1", 0.0E-1);
        assertion("0.1E-1", 0.1E-1);
        assertion("1.0E-1", 1.0E-1);
        assertion("01.01E-1", 01.01E-1);
        assertion("10.10E-1", 10.10E-1);
        assertion("123456789.123456789E-1", 123456789.123456789E-1);

        assertion("0.0E10", 0.0E10);
        assertion("0.1E10", 0.1E10);
        assertion("1.0E10", 1.0E10);
        assertion("01.01E10", 01.01E10);
        assertion("10.10E10", 10.10E10);
        assertion("123456789.1234567891E10", 123456789.123456789E10);

        assertion("0.0E-10", 0.0E-10);
        assertion("0.1E-10", 0.1E-10);
        assertion("1.0E-10", 1.0E-10);
        assertion("01.01E-10", 01.01E-10);
        assertion("10.10E-10", 10.10E-10);
        assertion("123456789.123456789E-10", 123456789.123456789E-10);
    }

    SUBCASE("Failure Modes") {
        assertion("1e1000000",
                  Error{.invalid_range = {0, 9}, .kind = Error::Kind::ConstantTooLarge});
    }
}

TEST_CASE("Constants") {
    Asserter assertion = SpecBuilder{
        .constants =
            Defaults::kBasicConstants +
            SpecFor<Constant>{
                {"I", 1.}, {"II", 2.}, {"III", 3.}, {"IV", 4.}, {"V", 5}, {"MCMLXXXIV", 1984.}},
    };

    SUBCASE("Constant Usage") {
        assertion("pi", M_PI);
        assertion("e", M_E);

        assertion("I", 1.);
        assertion("II", 2.);
        assertion("III", 3.);
        assertion("IV", 4.);
        assertion("V", 5.);
        assertion("MCMLXXXIV", 1984.);
    }

    SUBCASE("Failure Modes") {
        assertion("asd", Error{.invalid_range = {0, 3}, .kind = Error::Kind::UnknownIdentifier});
    }
}

TEST_CASE("Operator Precedence") {
    Asserter assertion = SpecBuilder{
        .unary_ops = {{"~", {.func = [](auto) { return 20.; }, .precedence = 2}}},
        .binary_ops = {{"@", {.func = [](auto, auto) { return 10.; }, .precedence = 1}},
                       {"#", {.func = [](auto, auto) { return 30.; }, .precedence = 3}}},
    };

    assertion("0 @ 0", 10.);
    assertion("0 # 0", 30.);

    assertion("0 # 0 @ 0", 10.);
    assertion("0 @ 0 # 0", 10.);

    assertion("~0", 20.);
    assertion("~0 @ 0", 10.);
    assertion("~0 # 0", 20.);
    assertion("~0 @ ~0", 10.);
    assertion("~0 # ~0", 20.);
}

TEST_CASE("Expression Evaluation Order") {
    Asserter assertion = SpecBuilder{
        .binary_ops = {
            {"<-", {.func = std::minus<double>{}, .left_associative = true, .precedence = 2}},
            {"->", {.func = std::minus<double>{}, .left_associative = false, .precedence = 2}},
            {"@", {.func = std::minus<double>{}, .precedence = 1}},
            {"#", {.func = std::minus<double>{}, .precedence = 3}},
        }};

    SUBCASE("Associativity ") {
        assertion("1 <- 2 <- 3", (1. - 2.) - 3.);
        assertion("1 -> 2 -> 3", 1. - (2. - 3.));
    }

    SUBCASE("Associativity and Precedence") {
        assertion("1 <- 2 <- 3 @ 4", ((1. - 2.) - 3.) - 4.);
        assertion("1 <- 2 <- 3 # 4", (1. - 2.) - (3. - 4.));

        assertion("1 -> 2 -> 3 @ 4", (1. - (2. - 3.)) - 4.);
        assertion("1 -> 2 -> 3 # 4", 1. - (2. - (3. - 4.)));

        assertion("4 @ 1 <- 2 <- 3", 4. - ((1. - 2.) - 3.));
        assertion("4 # 1 <- 2 <- 3", ((4. - 1.) - 2.) - 3.);

        assertion("4 @ 1 -> 2 -> 3", 4. - (1. - (2. - 3.)));
        assertion("4 # 1 -> 2 -> 3", (4. - 1.) - (2. - 3.));
    }

    SUBCASE("Failure Modes") {
        assertion("1 @", Error{.invalid_range = {3, 3}, .kind = Error::Kind::ValueExpected},
                  {Error{.invalid_range = {2, 2}, .kind = Error::Kind::ValueExpected}});
        assertion("@ 1", Error{.invalid_range = {0, 1}, .kind = Error::Kind::ValueExpected});
    }
}

TEST_CASE("Function Calls") {
    Asserter assertion = SpecBuilder{
        .unary_ops = Defaults::kNegateUnaryOp,
        .binary_ops = Defaults::kArithmeticBinaryOps,
        .unary_funs =
            {
                {"a", {.func = [](auto a) { return a + 10.; }}},
                {"asd", {.func = [](auto a) { return a + 20.; }}},
                {"asdaaaasssssdsddasdasd", {.func = [](auto a) { return a + 30.; }}},
                {"bcd", {.func = [](auto a) { return a + 40.; }}},
            },
        .binary_funs =
            {
                {"aa", {.func = [](auto a, auto b) { return a + b * 2 + 10.; }}},
                {"aassdd", {.func = [](auto a, auto b) { return a + b * 2 + 20.; }}},
                {"ddds1234q", {.func = [](auto a, auto b) { return a + b * 2 + 30.; }}},
                {"almafa", {.func = [](auto a, auto b) { return a + b * 2 + 40.; }}},
            },
    };

    SUBCASE("Simple Calls") {
        assertion("a(1)", 11.);
        assertion("asd(1)", 21.);
        assertion("asdaaaasssssdsddasdasd(1)", 31.);
        assertion("bcd(1)", 41.);
        assertion("aa(1, 2)", 15.);
        assertion("aassdd(1, 2)", 25.);
        assertion("ddds1234q(1, 2)", 35.);
        assertion("almafa(1, 2)", 45.);
    }

    SUBCASE("Calls Containing Expressions") {
        assertion("a((1))", 11.);
        assertion("asd(3 * 1 + 4)", 27.);
        assertion("bcd((1 + 2) * 3)", 49.);
        assertion("aa((1 + 2), 4 / (2))", 17.);
        assertion("aassdd(3 * 1 + 4, (1 + 2))", 33.);
        assertion("aassdd(3 * 1 + 4, (1 + 2))", 33.);
    }

    SUBCASE("Calls Inside Expressions") {
        assertion("a(1) - 1", 10.);
        assertion("1 - a(1)", -10.);
        assertion("1 - a(1) - 1", -11.);
        assertion("(1 - a(1)) - 1", -11.);
        assertion("1 - (a(1) - 1)", -9.);

        assertion("aa(1, 2) - 1", 14.);
        assertion("1 - aa(1, 2)", -14.);
        assertion("1 - aa(1, 2) - 1", -15.);
        assertion("(1 - aa(1, 2)) - 1", -15.);
        assertion("1 - (aa(1, 2) - 1)", -13.);

        assertion("1 - aa((2 + 3) * 4, -1) * 2", -55.);
    }

    SUBCASE("Nested Calls") {
        assertion("a(a(1))", 21.);
        assertion("aa(a(1), a(1))", 43.);
    }
}

TEST_CASE("Measures") {
    Asserter assertion = SpecBuilder{
        .unary_ops = Defaults::kNegateUnaryOp,
        .binary_ops = Defaults::kArithmeticBinaryOps,
        .unary_funs = Defaults::kBasicUnaryFuns,
        .measures = {Defaults::kLinearMeasure,
                     {"time",
                      {
                          {"msec", 1e-3},
                          {"sec", 1.},
                          {"min", 60.},
                          {"h", 60 * 60},
                      }}},
    };

    SUBCASE("Apply to Value") {
        assertion("1 km", 1e3);
        assertion("1 m", 1.);
        assertion("1 dm", 1e-1);
        assertion("1 cm", 1e-2);
        assertion("1 mm", 1e-3);

        assertion("1 msec", 1e-3);
        assertion("1 sec", 1.);
        assertion("1 min", 60.);
        assertion("1 h", 60. * 60.);

        assertion("(1 + 1) km", 2e3);
        assertion("abs(-1) cm", 1e-2);
        assertion("-1 dm", -0.1);
    }

    SUBCASE("Calculate with Different Units of the Same Measure") {
        assertion("1 m + 1 km", 1001.);
        assertion("1 km + 1 m", 1001.);

        assertion("1 m + 1", 2.);
        assertion("1 + 1m", 2.);
        assertion("1 km + 1", 1001.);
        assertion("1 + 1 km", 1001.);

        assertion("1 km * 1 km", 1e6);
    }
}

TEST_CASE("Arithmetic Examples") {
    Asserter assertion = SpecBuilder{
        .unary_ops = Defaults::kNegateUnaryOp,
        .binary_ops = Defaults::kArithmeticBinaryOps,
    };

    SUBCASE("Basic Binary Operators") {
        assertion("1 + 2", 1. + 2.);
        assertion("1 - 2", 1. - 2.);
        assertion("2 * 3", 2. * 3.);
        assertion("2 / 3", 2. / 3.);

        assertion("2 * 3 + 4", 2. * 3. + 4.);
        assertion("2 - 3 / 4", 2. - 3. / 4.);

        assertion("2 * 3 + 4 / 5", 2. * 3. + 4. / 5.);
        assertion("2 + 3 * 4 - 5", 2. + 3. * 4. - 5.);

        assertion("1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1", 10.);
    }

    SUBCASE("Basic Unary and Binary Operators") {
        assertion("-0", -0.);
        assertion("-1", -1.);

        assertion("- -1", 1.);
        assertion("- - - - - - - - - - -1", -1.);
        assertion("- - - - - - - - - - - -1", 1.);

        assertion("-2 - 3", -2. - 3.);
        assertion("2 - -3", 2. - -3.);
        assertion("-2 - -3", -2. - -3.);

        assertion("-2 * 3", -2. * 3.);
        assertion("2 * -3", 2. * -3.);
        assertion("-2 * -3", -2. * -3.);

        assertion("-2 * -3 - -2  / -3", -2. * -3. - -2. / -3.);
    }

    SUBCASE("Parenthesis with Binary Operators") {
        assertion("(1)", 1.);
        assertion("((1))", 1.);
        assertion("((((((((((1))))))))))", 1.);
        assertion("(123.456e-7)", 123.456e-7);
        assertion("((123.456e-7))", 123.456e-7);

        assertion("(1 + 2)", 1. + 2.);
        assertion("(1) + (2)", 1. + 2.);
        assertion("((1) + (2))", 1. + 2.);

        assertion("(1 + 2) * 3", (1. + 2.) * 3.);
        assertion("((1 + 2) * 3)", (1. + 2.) * 3.);
        assertion("1 + (2 * 3)", 1. + (2. * 3));
        assertion("(1 + (2 * 3))", 1. + (2. * 3));

        assertion("(2 * 3) + (4 / 5)", (2. * 3.) + (4. / 5.));
        assertion("2 * (3 + 4) / 5", 2. * (3. + 4.) / 5.);
        assertion("((2 * 3) + 4) / 5", ((2. * 3.) + 4.) / 5.);
        assertion("2 * (3 + (4 / 5))", 2. * (3. + (4. / 5.)));
        assertion("((2 * 3) + (4 / 5))", (2. * 3.) + (4. / 5.));
        assertion("(2 * (3 + 4) / 5)", 2. * (3. + 4.) / 5.);
        assertion("(((2 * 3) + 4) / 5)", ((2. * 3.) + 4.) / 5.);
        assertion("(2 * (3 + (4 / 5)))", 2. * (3. + (4. / 5.)));

        assertion("(2 + 3) * (4 - 5)", (2. + 3.) * (4. - 5.));
        assertion("2 + (3 * 4) - 5", 2. + (3. * 4.) - 5.);
        assertion("((2 + 3) * 4) - 5", ((2. + 3.) * 4.) - 5.);
        assertion("2 + (3 * (4 - 5))", 2. + (3. * (4. - 5.)));
        assertion("((2 + 3) * (4 - 5))", (2. + 3.) * (4. - 5.));
        assertion("(2 + (3 * 4) - 5)", 2. + (3. * 4.) - 5.);
        assertion("(((2 + 3) * 4) - 5)", ((2. + 3.) * 4.) - 5.);
        assertion("(2 + (3 * (4 - 5)))", 2. + (3. * (4. - 5.)));
    }

    SUBCASE("Parenthesis with Unary and Binary Operators") {
        assertion("-(0)", -0.);
        assertion("(-0)", -0.);
        assertion("(-(0))", -0.);
        assertion("-(1)", -1.);
        assertion("(-1)", -1.);
        assertion("(-(1))", -1.);

        assertion("--(1)", - -1.);
        assertion("-(-1)", - -1.);
        assertion("(--1)", - -1.);
        assertion("-(-(1))", - -1.);
        assertion("(-(-1))", - -1.);

        assertion("-(-(-(-(-(-(-(-(-(-(-(1)))))))))))", -1.);
        assertion("-(--(---(-----(1))))", -1.);

        assertion("-(2 * -3)", -(2. * -3.));
        assertion("(-2) + -3", (-2.) + -3.);
        assertion("-2 - (-3)", -2. - (-3.));
        assertion("(-(2) / -3)", (-2.) / -3.);

        assertion("-(2) * (-3) - -(-2  / -3)", -(2.) * (-3.) - -(-2. / -3.));
    }
}
