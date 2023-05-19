#include "DebugInspector.h"
#include <Wt/WTree.h>
#include <Wt/WTreeTableNode.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WBorderLayout.h>

#include <algorithm>

DebugInspector::DebugInspector()
{
	auto vbox = this->setLayout(Wt::cpp14::make_unique<Wt::WBorderLayout>());
	

	p_treeTable = vbox->addWidget(std::make_unique<Wt::WTreeTable>(), Wt::LayoutPosition::Center);

	p_treeTable->resize(650, 200);
	p_treeTable->tree()->setSelectionMode(Wt::SelectionMode::Extended);
	p_treeTable->addColumn("Yuppie Factor", 125);
	p_treeTable->addColumn("# Holidays", 125);
	p_treeTable->addColumn("Favorite Item", 125);


	auto root = std::make_unique<Wt::WTreeTableNode>("Variables");
	p_treeTable->setTreeRoot(std::move(root), "Emweb Organigram");

	auto group = std::make_unique<Wt::WTreeTableNode>("Upper Management");
	Wt::WTreeTableNode* group_ = group.get();

	p_treeTable->treeRoot()->addChildNode(std::move(group));
	addNode(group_, "Chief Anything Officer", "-2.8", "20");
	addNode(group_, "Vice President of Parties", "13.57", "365");
	addNode(group_, "Vice President of Staplery", "3.42", "27");

	group = std::make_unique<Wt::WTreeTableNode>("Middle management");
	group_ = group.get();

	p_treeTable->treeRoot()->addChildNode(std::move(group));
	addNode(group_, "Boss of the house", "9.78", "35");
	addNode(group_, "Xena caretaker", "8.66", "10");

	group = std::make_unique<Wt::WTreeTableNode>("Actual Workforce");
	group_ = group.get();

	p_treeTable->treeRoot()->addChildNode(std::move(group));
	addNode(group_, "The Dork", "9.78", "22");
	addNode(group_, "The Stud", "8.66", "46");
	addNode(group_, "The Ugly", "13.0", "25");

	p_treeTable->treeRoot()->expand();
}

Wt::WTreeTableNode* DebugInspector::addNode(
	Wt::WTreeTableNode* parent, 
	const Wt::WString name,
	const Wt::WString datatype, 
	const Wt::WString value)
{
	auto node = std::make_unique<Wt::WTreeTableNode>(name);
	auto node_ = node.get();
	parent->addChildNode(std::move(node));
	node_->setColumnWidget(1, std::make_unique<Wt::WText>(datatype));
	node_->setColumnWidget(2, std::make_unique<Wt::WText>(value));
	return node_;
}

void DebugInspector::update(Bytecode& bytecode)
{
	for (Wt::WTreeNode* n : p_treeTable->treeRoot()->childNodes()) {
		p_treeTable->treeRoot()->removeChildNode(n);
	}

	for (Datatype& variable : bytecode.getVariables()) {
		if (variable.getVariableType() == Datatype::EVariableTypes::VARIABLE) {
			const char* datatype = variable.getDatatype();
			const void* address = variable.getAddress();

			std::string value;

			if (address == NULL) {
				value = "N/A";
			}
			else {
				if (datatype[0] == 'i') {
					value = std::to_string(*static_cast<const long long int*>(address));
				}
				else if (datatype[0] == 'f') {
					value = std::to_string(*static_cast<const long double*>(address));
				}
				else if (datatype[0] == 's') {
					value = *static_cast<const std::string*>(address);
				}
				else if (datatype[0] == 'b') {
					value = (*static_cast<const unsigned char*>(address)) ? "true" : "false";
				}
				else if (datatype[0] == 'a') {
					value = "Array";
				}
			} // ~if (address == NULL)

			Wt::WTreeTableNode* new_node = addNode(
				p_treeTable->treeRoot(),
				Wt::WString(variable.getScope()) + ":" + variable.getName(),
				Wt::WString(variable.getDatatype()),
				Wt::WString(value)
			);

			if (address != NULL && datatype[0] == 'a') {
				getArrayElements(new_node, static_cast<const Array*>(address));
			}
		}
	}
}

void DebugInspector::getArrayElements(Wt::WTreeTableNode* parent, const Array* array)
{
	for (ArrayElement* el : array->getElements()) {
		ValuePointer& v = el->getValue();
		std::string val_str;

		if (   (v.datatype[0] == 'i') 
			|| (v.datatype[0] == 'f')
			|| (v.datatype[0] == 's')
			|| (v.datatype[0] == 'b')
			)
		{
			val_str = el->valueToString();
		}
		else if (v.datatype[0] == 'a') {
			val_str = "Array";
		}

		Wt::WTreeTableNode* new_node = addNode(
			parent,
			Wt::WString(el->indexesToString()),
			Wt::WString(el->getValue().datatype),
			Wt::WString(val_str)
		);

		if (v.datatype[0] == 'a') {
			getArrayElements(new_node, static_cast<const Array*>(el->getValue().pvalue));
		}
	}
}