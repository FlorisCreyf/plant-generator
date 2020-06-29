/* Copyright 2020 Floris Creyf
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xml_writer.h"

using std::string;

XMLWriter::XMLWriter(const char *filename)
{
	this->depth = 0;
	this->file.open(filename, std::ios::out);
}

XMLWriter::~XMLWriter()
{
	this->file.close();
}

void XMLWriter::operator>>(string tag)
{
	this->file << string(this->depth*2, ' ') << tag << std::endl;
	this->depth++;
}

void XMLWriter::operator<<(string tag)
{
	this->depth--;
	this->file << string(this->depth*2, ' ') << tag << std::endl;
}

void XMLWriter::operator+=(string tag)
{
	this->file << string(this->depth*2, ' ') << tag << std::endl;
}
