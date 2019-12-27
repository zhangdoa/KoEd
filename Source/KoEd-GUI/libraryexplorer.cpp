#include "libraryexplorer.h"
#include "../../Source/KoEd-Core/RegManager.h"
using namespace KoEd;

LibraryExplorer::LibraryExplorer(QWidget* parent) : QTreeWidget(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            SIGNAL(customContextMenuRequested(const QPoint&)),
            SLOT(onCustomContextMenuRequested(const QPoint&)));
}

void LibraryExplorer::buildTree()
{
    m_rootItem = new QTreeWidgetItem(this);
    m_rootItem->setText(0, "Kontakt Libraries");
    this->addTopLevelItem(m_rootItem);

    auto l_libInfo = GetLibInfos();

    for (auto& i : l_libInfo)
    {
        QTreeWidgetItem* l_item = new QTreeWidgetItem();

        l_item->setText(0, i.name.c_str());
        addChild(m_rootItem, l_item);
    }
}

void LibraryExplorer::initialize(PropertyEditor* propertyEditor)
{
    m_propertyEditor = propertyEditor;

    buildTree();

    this->expandAll();
}

void LibraryExplorer::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QTreeWidget::selectionChanged(selected, deselected);
    QList<QTreeWidgetItem*> selectedItems = this->selectedItems();
    QTreeWidgetItem* item;
    if (selectedItems.count() != 0)
    {
        item = selectedItems[0];
        if (item != m_rootItem)
        {
            if (m_propertyEditor)
            {
                auto l_libObject = item->data(1, Qt::UserRole).value<void*>();
                m_propertyEditor->edit(l_libObject);
            }
        }
    }
}

void LibraryExplorer::addChild(QTreeWidgetItem *parent, QTreeWidgetItem *child)
{
    parent->addChild(child);
}

void LibraryExplorer::startRename()
{
    auto l_items = this->selectedItems();
    QTreeWidgetItem* item;
    if (l_items.count() != 0)
    {
        item = l_items[0];
        m_currentEditingItem = item;
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        this->editItem(item);
        connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(endRename()));
    }
}

void LibraryExplorer::endRename()
{
    (m_currentEditingItem->text(0).toStdString() + "/").c_str();

    m_currentEditingItem->setFlags(m_currentEditingItem->flags() & ~Qt::ItemIsEditable);
    disconnect(this, SIGNAL(itemSelectionChanged()), this, SLOT(endRename()));
}

void LibraryExplorer::addLibrary()
{
    QTreeWidgetItem* l_item = new QTreeWidgetItem();

    l_item->setText(0, "New Kontakt Library");

    addChild(m_rootItem, l_item);
    this->setCurrentItem(l_item);
    startRename();
}

void LibraryExplorer::deleteLibrary()
{
    auto l_items = this->selectedItems();
    QTreeWidgetItem* item;
    if (l_items.count() != 0)
    {
        item = l_items[0];
        item->parent()->removeChild(item);
    }
}

void LibraryExplorer::onCustomContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);

    if (item)
    {
        showContextMenu(item, viewport()->mapToGlobal(pos));
    }
    else
    {
        showGeneralMenu(viewport()->mapToGlobal(pos));
    }
}

void LibraryExplorer::showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos)
{
    QMenu menu;

    if (item != m_rootItem)
    {
        menu.addAction("Library context menu");
        menu.addAction("Rename", this, SLOT(startRename()));

        menu.addAction("Delete", this, SLOT(deleteLibrary()));
    }
    else
    {
        menu.addAction("Add Library", this, SLOT(addLibrary()));
    }

    menu.exec(globalPos);
}

void LibraryExplorer::showGeneralMenu(const QPoint &globalPos)
{
    QMenu menu;
    menu.addAction("General menu");
    menu.exec(globalPos);
}
