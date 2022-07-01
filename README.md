![Unit Tests](https://github.com/bagyibarna/measure-calculator/workflows/UnitTest/badge.svg)

# A Fully Customizable, Interactive Calculator Library

`measure-calculator` is a calculator library designed for continous user interaction:
 - supports measures (`m`, `ft`, etc.)
 - heavy customizability
 - good error reporting
 - optimized for repeated evaluation of small to medium expressions
     - no allocations during evaluation
 - small codebase

## Example usage:

```cpp
    using namespace Calc;

    // create the Spec once
    auto spec = SpecBuilder {
        .binary_ops = Defaults::kArithmeticBinaryOps,
        .constants = Defaults::kBasicConstants,
        .measures = { Defaults::kLinearMeasure },
    }.Build();

    if (auto* error = std::get_if<SpecBuilder::Error>(spec)) {
        // handle error
    }

    // use the same Spec for evaluation many times
    auto result = Evaluate(spec, "1km + 2 * 100 m * pi");
    
    if (auto* error = std::get_if<Error>(result)) {
        // handle error
    }

    //
    return std::get<double>(result);
```
