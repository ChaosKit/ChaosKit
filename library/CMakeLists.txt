add_library(library
        coloring_methods/Distance.h
        coloring_methods/Noop.h
        coloring_methods/SingleColor.h
        formulas/DeJong.h
        formulas/Drain.h
        formulas/Linear.h
        formulas/Quadratic.h
        formulas/Sinusoidal.h
        formulas/Spherical.h
        formulas/Tinkerbell.h
        formulas/Trigonometric.h
        ColoringMethod.h
        ColoringMethodType.h
        Formula.h
        FormulaType.h
        util.cpp util.h)
target_link_libraries(library PUBLIC ast PRIVATE magic_enum)
