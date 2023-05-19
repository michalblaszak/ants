#pragma once
#include <Wt/WContainerWidget.h>
#include <Wt/WtreeTable.h>
#include "bytecode.h"

class DebugInspector :
    public Wt::WContainerWidget
{
public:
    DebugInspector();

    Wt::WTreeTableNode* addNode(
        Wt::WTreeTableNode* parent,
        const Wt::WString name,
        const Wt::WString datatype,
        const Wt::WString value);
    void update(Bytecode& bytecode);

private:
    Wt::WTreeTable* p_treeTable;

    void getArrayElements(Wt::WTreeTableNode* parent, const Array* array);
};

