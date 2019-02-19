add_library(library
        Formula.h
        FormulaType.h
        DeJong.h
        Drain.h
        Linear.h
        util.cpp util.h)
target_link_libraries(library PUBLIC ast better_enums)
