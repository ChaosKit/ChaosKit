add_library(core_models
        BlendModel.cpp BlendModel.h
        DocumentModel.cpp DocumentModel.h
        FormulaModel.cpp FormulaModel.h
        Model.h
        ModelCollection.h
        SystemModel.cpp SystemModel.h
        )
target_link_libraries(core_models PUBLIC core_structures)
