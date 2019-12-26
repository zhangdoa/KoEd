#ifndef PROPERTYEDITOR_H
#define PROPERTYEDITOR_H

#include <QWidget>

class PropertyEditor : public QWidget
{
	Q_OBJECT
public:
    explicit PropertyEditor(QWidget *parent = nullptr);
	void initialize();
	void clear();

    void edit(void* rhs);
	void remove();

signals:

public slots:
};

#endif // PROPERTYEDITOR_H
