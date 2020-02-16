add_library(library
        coloring_methods/Distance.h
        coloring_methods/Noop.h
        coloring_methods/SingleColor.h
        ColoringMethod.h
        ColoringMethodType.h
        DeJong.h
        Drain.h
        Formula.h
        FormulaType.h
        Linear.h
        util.cpp util.h
        )
target_link_libraries(library PUBLIC ast better_enums)
