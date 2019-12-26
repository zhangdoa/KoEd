#ifndef LIBRARYEXPLORER_H
#define LIBRARYEXPLORER_H

#include <QTreeWidget>
#include <QMenu>
#include <QShortcut>
#include "propertyeditor.h"

class LibraryExplorer : public QTreeWidget
{
	Q_OBJECT
public:
    explicit LibraryExplorer(QWidget *parent = nullptr);

    void initialize(PropertyEditor* propertyEditor);

protected:
	virtual void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private slots:
	void onCustomContextMenuRequested(const QPoint& pos);
	void showContextMenu(QTreeWidgetItem* item, const QPoint& globalPos);
	void showGeneralMenu(const QPoint& globalPos);

	void startRename();
	void endRename();

    void addLibrary();
    void deleteLibrary();

private:
	void addChild(QTreeWidgetItem* parent, QTreeWidgetItem* child);

    PropertyEditor* m_propertyEditor;

	QTreeWidgetItem* m_rootItem;
	QTreeWidgetItem* m_currentEditingItem;
	void buildTree();
};

#endif // LIBRARYEXPLORER_H
