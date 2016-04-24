/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software(0), you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation(0), either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY(0), without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program(0), if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "titanic/pet_control/pet_text.h"

namespace Titanic {

CPetText::CPetText(uint count) :
		_stringsMerged(false), _maxCharsPerLine(-1), _lineCount(0),
		_fontNumber1(-1), _field3C(0), _field40(0), _field44(0),
		_backR(0xff), _backG(0xff), _backB(0xff), 
		_textR(0), _textG(0), _textB(200),
		_fontNumber2(0), _field64(0), _field68(0), _field6C(0),
		_hasBorder(true), _field74(0), _field78(0), _field7C(0) {
	setupArrays(count);
}

void CPetText::setupArrays(int count) {
	freeArrays();
	if (count < 10 || count > 60)
		count = 10;
	_array.resize(count);
}

void CPetText::freeArrays() {
	_array.clear();
}

void CPetText::setup() {
	for (int idx = 0; idx < (int)_array.size(); ++idx) {
		_array[idx]._string1.clear();
		setArrayStr2(idx, _textR, _textG, _textB);
		_array[idx]._string3.clear();
	}

	_lineCount = 0;
	_stringsMerged = false;
}

void CPetText::setArrayStr2(uint idx, int val1, int val2, int val3) {
	char buffer[6];
	if (!val1)
		val1 = 1;
	if (!val2)
		val2 = 1;
	if (!val3)
		val3 = 1;

	buffer[0] = 27;
	buffer[1] = val1;
	buffer[2] = val2;
	buffer[3] = val3;
	buffer[4] = 27;
	buffer[5] = '\0';
	_array[idx]._string2 = buffer;
}

void CPetText::load(SimpleFile *file, int param) {
	if (!param) {
		int var1 = file->readNumber();
		int var2 = file->readNumber();
		uint count = file->readNumber();
		_bounds.left = file->readNumber();
		_bounds.top = file->readNumber();
		_bounds.right = file->readNumber();
		_bounds.bottom = file->readNumber();
		_field3C = file->readNumber();
		_field40 = file->readNumber();
		_field44 = file->readNumber();
		_backR = file->readNumber();
		_backG = file->readNumber();
		_backB = file->readNumber();
		_textR = file->readNumber();
		_textG = file->readNumber();
		_textB = file->readNumber();
		_hasBorder = file->readNumber() != 0;
		_field74 = file->readNumber();

		warning("TODO: CPetText::load %d,%d", var1, var2);
		assert(_array.size() >= count);
		for (uint idx = 0; idx < count; ++idx) {
			_array[idx]._string1 = file->readString();
			_array[idx]._string2 = file->readString();
			_array[idx]._string3 = file->readString();
		}	
	}
}

void CPetText::draw(CScreenManager *screenManager) {
	Rect tempRect = _bounds;

	if (_hasBorder) {
		// Create border effect
		// Top edge
		tempRect.bottom = tempRect.top + 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Bottom edge
		tempRect.top = _bounds.bottom - 1;
		tempRect.bottom = _bounds.bottom;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Left edge
		tempRect = _bounds;
		tempRect.right = tempRect.left + 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);

		// Right edge
		tempRect = _bounds;
		tempRect.left = tempRect.right - 1;
		screenManager->fillRect(SURFACE_BACKBUFFER, &tempRect, _backR, _backG, _backB);
	}

	draw2(screenManager);

	tempRect = _bounds;
	tempRect.grow(-2);
	screenManager->setFontNumber(_fontNumber2);

//	int var14 = 0;
//	screenManager->writeLines(0, &var14, _field74, )
	warning("TODO: CPetText_Draw");
	screenManager->setFontNumber(_fontNumber1);
}

void CPetText::mergeStrings() {
	if (!_stringsMerged) {
		_lines.clear();

		for (int idx = 0; idx < _lineCount; ++idx) {
			CString line = _array[idx]._string2 + _array[idx]._string3 +
				_array[idx]._string1 + "\n";
			_lines += line;

		}

		_stringsMerged = true;
	}
}

void CPetText::resize(uint count) {
	if (!count || _array.size() == count)
		return;
	_array.clear();
	_array.resize(count);
}

void CPetText::setText(const CString &str) {
	setup();
	changeText(str);
}

void CPetText::changeText(const CString &str) {
	int lineSize = _array[_lineCount]._string1.size();
	int strSize = str.size();

	if (_maxCharsPerLine == -1) {
		// No limit on horizontal characters, so append string to current line
		_array[_lineCount]._string1 += str;
	} else if ((lineSize + strSize) <= _maxCharsPerLine) {
		// New string fits into line, so add it on
		_array[_lineCount]._string1 += str;
	} else {
		// Only add part of the str up to the maximum allowed limit for line
		_array[_lineCount]._string1 += str.left(_maxCharsPerLine - lineSize);
	}
	
	updateStr3(_lineCount);
	_stringsMerged = false;
}

void CPetText::setColor(int val1, uint col) {
	warning("CPetText::setColor");
}

void CPetText::setColor(uint col) {
	_textR = col & 0xff;
	_textG = (col >> 8) & 0xff;
	_textB = (col >> 16) & 0xff;
}

void CPetText::setMaxCharsPerLine(int maxChars) {
	if (maxChars >= -1 && maxChars < 257)
		_maxCharsPerLine = maxChars;
}

void CPetText::updateStr3(int lineNum) {
	if (_field64 > 0 && _field68 > 0) {
		char line[5];
		line[0] = line[3] = 26;
		line[1] = _field64;
		line[2] = _field68;
		line[4] = '\0';
		_array[lineNum]._string3 = CString(line);
		
		_stringsMerged = false;
		_field64 = _field68 = 0;
	}
}

void CPetText::draw2(CScreenManager *screenManager) {

}

} // End of namespace Titanic
