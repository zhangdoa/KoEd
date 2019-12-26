#include "propertyeditor.h"

PropertyEditor::PropertyEditor(QWidget *parent) : QWidget(parent)
{
}

void PropertyEditor::initialize()
{
    //this->layout()->setAlignment(Qt::AlignTop);
}

void PropertyEditor::clear()
{
}

void PropertyEditor::edit(void *rhs)
{
	remove();
}

void PropertyEditor::remove()
{
}
