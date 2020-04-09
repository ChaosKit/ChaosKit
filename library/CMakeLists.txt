add_library(library
        coloring_methods/Distance.h
        coloring_methods/Noop.h
        coloring_methods/SingleColor.h
        formulas/DeJong.h
        formulas/Drain.h
        formulas/Linear.h
        ColoringMethod.h
        ColoringMethodType.h
        Formula.h
        FormulaType.h
        util.cpp util.h
        )
target_link_libraries(library PUBLIC ast better_enums)
