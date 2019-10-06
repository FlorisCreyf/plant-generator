/* Plant Genererator
 * Copyright (C) 2019  Floris Creyf
 *
 * Plant Genererator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Plant Genererator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "keymap.h"
#include <QtXml>
#include <QFile>

void KeyMap::loadFromXMLFile(const char *filename)
{
	QDomDocument xml;
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		printf("Could not open keymap file.");
		return;
	}
	xml.setContent(&file);
	file.close();

	QDomElement root = xml.documentElement();
	QString tagname = root.tagName();

	QDomElement element = root.firstChild().toElement();
	while (!element.isNull()) {
		Binding binding = {};
		QString name;
		QString key;

		QDomNodeList nodes;
		nodes = element.elementsByTagName(tr("name"));
		if (nodes.size() > 0)
			name = nodes.at(0).firstChild().toText().data();
		nodes = element.elementsByTagName(tr("key"));
		if (nodes.size() > 0)
			key = nodes.at(0).firstChild().toText().data();

		QStringList list = key.split("+");
		for (int i = 0; i < list.size(); i++) {
			QString part = list.at(i);
			if (part == tr("CTRL"))
			 	binding.ctrl = true;
			else if (part == tr("ALT"))
				binding.alt = true;
			else if (part == tr("SHIFT"))
				binding.shift = true;
			else if (part == tr("DEL"))
				binding.key = Qt::Key_Delete;
			else
				binding.key = part[0].toLatin1();
		}

		if (!name.isNull() && !key.isNull())
			bindings[binding] = name;

		element = element.nextSibling().toElement();
	}
}

QString KeyMap::getBinding(unsigned key, bool ctrl, bool shift, bool alt)
{
	Binding binding = {};
	binding.key = key;
	binding.ctrl = ctrl;
	binding.shift = shift;
	binding.alt = alt;
	return bindings[binding];
}

QString KeyMap::toString(KeyMap::Binding binding) const
{
	QString str = "";
	if (binding.ctrl)
		str += "CTRL+";
	if (binding.shift)
		str += "SHIFT+";
	if (binding.alt)
		str += "ALT+";

	if (binding.key == Qt::Key_Delete)
		return str + tr("DEL");
	else
		return str + binding.key;
}

std::map<KeyMap::Binding, QString> KeyMap::getBindings() const
{
	return bindings;
}
