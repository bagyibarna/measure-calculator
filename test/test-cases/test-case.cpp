#include <doctest/doctest.h>

#define _USE_MATH_DEFINES
#include <cmath>

#include "measure-calculator/defaults.hpp"
#include "measure-calculator/measure-calculator.hpp"

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
                    std::optional<T> noWhitespaceExpected = std::nullopt) const {
        using PureT = std::remove_cvref_t<T>;
        auto result = Evaluate(spec, str);
        CHECK_UNARY(std::holds_alternative<PureT>(result));
        CHECK_EQ(std::get<PureT>(result), WrapForCheck(expected));

        std::string noWhitespace(str.begin(), str.end());
        noWhitespace.erase(std::remove_if(noWhitespace.begin(), noWhitespace.end(), ::isspace),
                           noWhitespace.end());

        auto noWhitespaceResult = Evaluate(spec, noWhitespace);
        CHECK_UNARY(std::holds_alternative<PureT>(noWhitespaceResult));
        CHECK_EQ(std::get<PureT>(noWhitespaceResult),
                 WrapForCheck(noWhitespaceExpected.value_or(expected)));
    }
};

const SpecBuilder kDefaultBuilder{
    .unaryOps = Defaults::kNegateUnaryOp,
    .binaryOps = Defaults::kArithmeticBinaryOps,
    .unaryFuns = SpecUnion(Defaults::kBasicUnaryFuns, Defaults::kExponentialUnaryFuns,
                           Defaults::kTrigonometricUnaryFuns),
    .binaryFuns = Defaults::kBasicBinaryFuns,
    .constants = Defaults::kBasicConstants,
    .measures = {Defaults::kLinearMeasure},
};

TEST_CASE("Spec Failure Modes") {
    using SBError = SpecBuilder::Error;

    const auto buildsTo = [](SBError error, SpecBuilder spec) {
        auto result = std::move(spec).Build();
        CHECK_UNARY(std::holds_alternative<SBError>(result));
        CHECK_EQ(std::get<SBError>(result), error);
    };

    const auto dummyUnaryFunc = [](double d) { return d; };
    const auto dummyBinaryFunc = [](double d, double) { return d; };

    SUBCASE("Invalid Characters") {
        buildsTo(SBError::InvalidOperatorName, {
                                                   .unaryOps =
                                                       {
                                                           {"alma", {.func = dummyUnaryFunc}},
                                                       }
                                               });

        buildsTo(SBError::InvalidOperatorName, {
                                                   .binaryOps =
                                                       {
                                                           {"alma", {.func = dummyBinaryFunc}},
                                                       }
        });

        buildsTo(SBError::InvalidIdentifierName, {
                                                     .measures =
                                                         {
                                                             {"wat", {{"1", 1}}},
                                                         }
        });

        buildsTo(SBError::InvalidIdentifierName,
                 {.unaryFuns = {{"min(", {.func = dummyUnaryFunc}}}});
        buildsTo(SBError::InvalidIdentifierName,
                 {.binaryFuns = {{"*", {.func = dummyBinaryFunc}}}});
        buildsTo(SBError::InvalidIdentifierName, {.constants = {{"0", 0}}});
        buildsTo(SBError::InvalidIdentifierName, {.constants = {{" ", 0}}});
    }

    SUBCASE("Duplicates") {
        buildsTo(SBError::DuplicateOperator, {
                                                 .unaryOps =
                                                     {
                                                         {"+", {.func = dummyUnaryFunc}},
                                                         {"+", {.func = dummyUnaryFunc}},
                                                     }
                                             });
        buildsTo(SBError::DuplicateOperator, {
                                                 .binaryOps =
                                                     {
                                                         {"*", {.func = dummyBinaryFunc}},
                                                         {"*", {.func = dummyBinaryFunc}},
                                                     }
                                             });
        buildsTo(SBError::DuplicateOperator, {
                                                 .unaryOps =
                                                     {
                                                         {"*", {.func = dummyUnaryFunc}},
                                                     }
                                                 .binaryOps =
                                                     {
                                                         {"*", {.func = dummyBinaryFunc}},
                                                         {"*", {.func = dummyBinaryFunc}},
                                                     }
                                             });

        buildsTo(SBError::DuplicateIdentifier, {
                                                   .measures =
                                                       {
                                                           {"name", {{"alma", 1}}},
                                                           {"name", {{"alma", 2}}},
                                                       }
                                               });

        buildsTo(SBError::DuplicateIdentifier, {.measures =
                                                    {
                                                        {"name", {{"alma", 1}}},
                                                    }
                                                .unaryFuns = {{"alma", {.func = dummyUnaryFunc}}}});

        buildsTo(SBError::DuplicateIdentifier,
                 {.unaryFuns = {{"name", {.func = dummyUnaryFunc}}}, .constants = {{"name", 12}}});
    }

    SUBCASE("Invalid Measures") {
        buildsTo(SBError::ZeroMultiplier, {
                                              .measures =
                                                  {
                                                      {"name", {{"alma", 0}}},
                                                  }
                                          });

        buildsTo(SBError::NegativeMultiplier, {
                                                  .measures =
                                                      {
                                                          {"name", {{"alma", -1}}},
                                                      }
                                              });
    }
}

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
                  Error{.kind = Error::Kind::ConstantTooLarge, .invalidRange = {0, 9}});
    }
}

TEST_CASE("Constants") {
    Asserter assertion = SpecBuilder{
        .constants = SpecUnion(
            Defaults::kBasicConstants,
            SpecFor<Constant>{
                {"I", 1.}, {"II", 2.}, {"III", 3.}, {"IV", 4.}, {"V", 5}, {"MCMLXXXIV", 1984.}}),
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
        assertion("asd", Error{.kind = Error::Kind::UnknownIdentifier, .invalidRange = {0, 3}});
    }
}

TEST_CASE("Operator Precedence") {
    Asserter assertion = SpecBuilder{
        .unaryOps = {{"~", {.func = [](auto) { return 20.; }, .precedence = 2}}},
        .binaryOps = {{"@", {.func = [](auto, auto) { return 10.; }, .precedence = 1}},
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
        .binaryOps = {
            {"<-", {.func = std::minus<double>{}, .leftAssociative = true, .precedence = 2}},
            {"->", {.func = std::minus<double>{}, .leftAssociative = false, .precedence = 2}},
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
        assertion("1 @", Error{.kind = Error::Kind::ValueExpected, .invalidRange = {3, 3}},
                  {Error{.kind = Error::Kind::ValueExpected, .invalidRange = {2, 2}}});
        assertion("@ 1", Error{.kind = Error::Kind::ValueExpected, .invalidRange = {0, 1}});
    }
}

TEST_CASE("Function Calls") {
    Asserter assertion = SpecBuilder{
        .unaryOps = Defaults::kNegateUnaryOp,
        .binaryOps = Defaults::kArithmeticBinaryOps,
        .unaryFuns =
            {
                {"a", {.func = [](auto a) { return a + 10.; }}},
                {"asd", {.func = [](auto a) { return a + 20.; }}},
                {"asdaaaasssssdsddasdasd", {.func = [](auto a) { return a + 30.; }}},
                {"bcd", {.func = [](auto a) { return a + 40.; }}},
            },
        .binaryFuns =
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
        .unaryOps = Defaults::kNegateUnaryOp,
        .binaryOps = Defaults::kArithmeticBinaryOps,
        .unaryFuns = Defaults::kBasicUnaryFuns,
        .binaryFuns = Defaults::kBasicBinaryFuns,
        .measures = {Defaults::kLinearMeasure,
                     Defaults::kAngularMeasure,
                     {"time",
                      {
                          {"msec", 1e-3},
                          {"sec", 1.},
                          // min would collide with min(...)
                          {"h", 60 * 60},
                      }}},
    };

    SUBCASE("Apply to Value") {
        assertion("1 km", 1e3);
        assertion("1 m", 1.);
        assertion("1 dm", 1e-1);
        assertion("1 cm", 1e-2);
        assertion("1 mm", 1e-3);

        assertion("1 turn", 2 * Defaults::pi);
        assertion("1 rad", 1.);
        assertion("1 º", Defaults::pi / 180.);
        assertion("1 °", Defaults::pi / 180.);
        assertion("1 '", Defaults::pi / (180. * 60.));
        assertion("1 ''", Defaults::pi / (180. * 60. * 60));
        assertion("1 \"", Defaults::pi / (180. * 60. * 60));

        assertion("1 msec", 1e-3);
        assertion("1 sec", 1.);
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

    SUBCASE("Failure Modes") {
        assertion("1 km + 1 sec",
                  Error{.kind = Error::Kind::MeasureMismatch,
                        .invalidRange = {9, 12},
                        .secondaryInvalidRange = {2, 4}},
                  {Error{.kind = Error::Kind::MeasureMismatch,
                         .invalidRange = {5, 8},
                         .secondaryInvalidRange = {1, 3}}});
        assertion("max(1 km, 1 sec)",
                  Error{.kind = Error::Kind::MeasureMismatch,
                        .invalidRange = {12, 15},
                        .secondaryInvalidRange = {6, 8}},
                  {Error{.kind = Error::Kind::MeasureMismatch,
                         .invalidRange = {9, 12},
                         .secondaryInvalidRange = {5, 7}}});
    }
}

TEST_CASE("Arithmetic Examples") {
    Asserter assertion = SpecBuilder{
        .unaryOps = Defaults::kNegateUnaryOp,
        .binaryOps = Defaults::kArithmeticBinaryOps,
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

    SUBCASE("Failure Modes") {
        assertion("1 / 0", Error{.kind = Error::Kind::InfiniteValue, .invalidRange = {2, 3}},
                  {Error{.kind = Error::Kind::InfiniteValue, .invalidRange = {1, 2}}});
        assertion("1 / 0 + 3", Error{.kind = Error::Kind::InfiniteValue, .invalidRange = {2, 3}},
                  {Error{.kind = Error::Kind::InfiniteValue, .invalidRange = {1, 2}}});
        assertion("0 / 0", Error{.kind = Error::Kind::NotANumber, .invalidRange = {2, 3}},
                  {Error{.kind = Error::Kind::NotANumber, .invalidRange = {1, 2}}});
        assertion("0 / 0 + 3", Error{.kind = Error::Kind::NotANumber, .invalidRange = {2, 3}},
                  {Error{.kind = Error::Kind::NotANumber, .invalidRange = {1, 2}}});
    }
}

TEST_CASE("Posfix Binary ShortHand") {
    Asserter assertion = SpecBuilder{.unaryOps = Defaults::kNegateUnaryOp,
                                     .binaryOps = Defaults::kArithmeticBinaryOps,
                                     .unaryFuns = Defaults::kBasicUnaryFuns,
                                     .binaryFuns = Defaults::kBasicBinaryFuns,
                                     .usePostfixShorthand = true};

    SUBCASE("On Raw Values") {
        assertion("3 +", 3. + 3.);
        assertion("3 *", 3. * 3.);
        assertion("3 /", 3. / 3.);
    }

    SUBCASE("On Expressions") {
        assertion("abs(-3) +", 3. + 3.);
        assertion("min(3, 5) *", 3. * 3.);
        assertion("(1 + 2) +", 3. + 3.);
    }

    SUBCASE("Failure Modes") {
        assertion("3 - asd", Error{.kind = Error::Kind::UnknownIdentifier, .invalidRange = {4, 7}},
                  {Error{.kind = Error::Kind::UnknownIdentifier, .invalidRange = {2, 5}}});
        assertion("(3 -)", Error{.kind = Error::Kind::ValueExpected, .invalidRange = {4, 5}},
                  {Error{.kind = Error::Kind::ValueExpected, .invalidRange = {3, 4}}});
    }
}
