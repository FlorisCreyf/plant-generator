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

#ifndef KEY_MAP_H
#define KEY_MAP_H

#include <map>
#include <string>
#include <QObject>

class KeyMap : public QObject {
public:
	struct Binding {
		unsigned key;
		bool ctrl;
		bool shift;
		bool alt;
	};

	void loadFromXMLFile(const char *filename);
	QString getBinding(unsigned key, bool ctrl, bool shift, bool alt);
	QString toString(Binding binding) const;
	std::map<Binding, QString> getBindings() const;

private:
	std::map<Binding, QString> bindings;
};

inline bool operator<(const KeyMap::Binding &a, const KeyMap::Binding &b) {
	if (a.key != b.key)
		return a.key < b.key;
	else if (a.ctrl != b.ctrl)
		return a.ctrl;
	else if (a.shift != b.shift)
		return a.shift;
	else if (a.alt != b.alt)
		return b.alt;
	else
		return false;
}

#endif /* KEY_MAP_H */
