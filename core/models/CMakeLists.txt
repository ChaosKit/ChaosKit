add_library(core_models
        GlobalStore.cpp GlobalStore.h
        )
target_link_libraries(core_models PUBLIC core_structures state)
