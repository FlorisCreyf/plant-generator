/*
 * Copyright (C) 2016 Floris Creyf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef PROPERTY_BOX
#define PROPERTY_BOX

#include <QtWidgets>

class PropertyBox : public QWidget
{
public:
	PropertyBox(QWidget *parent = 0);
	QSize sizeHint() const;
};

#endif /* PROPERTY_BOX */
